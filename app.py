import copy
import hashlib
import threading
import time
import logging
import argparse
import os.path
from pathlib import Path
import shutil
import filecmp
import regex as re
from textwrap import dedent
import subprocess
import sys
STDLIB_RE = sys.modules['re']  # before iheartla replaces it with `regex`
from iheartla.la_tools.la_helper import *
from iheartla.la_tools.la_logger import LaLogger
from iheartla.la_tools.la_msg import LaMsg
import markdown
import markdown.extensions
from markdown.core import *


# Match string: <figure>***</figure>
FIGURE_BLOCK_RE = re.compile(
    dedent(r'''<figure(?P<property>[^\n>]*)>(?P<figure>.*?)</figure>'''),
    re.MULTILINE | re.DOTALL | re.VERBOSE
)
# Match string: <img src='' ***>
IMAGE_BLOCK_RE = re.compile(
    dedent(r'''<img\ (?P<before>[^\n>]*)src=(?P<quote>"|')(?P<src>[^\n'">]*)(?P=quote)(?P<after>[^\n>]*)>'''),
    re.MULTILINE | re.DOTALL | re.VERBOSE
)
# The height an <img> of an HTML figure gives its iframe: <img src='figure.html' height='900'>
IMAGE_HEIGHT_RE = re.compile(r'''\bheight=(?P<quote>"|'|)(?P<height>\d+)(?P=quote)''')
FIGURE_HEIGHT = 525
cached_figure_dict = {}
cached_figure_code = {}
# In Pyodide (the browser), there are no subprocesses or threads
IN_BROWSER = sys.platform == 'emscripten'
APP_DIR = os.path.dirname(os.path.abspath(__file__))
# The I❤️MESH runtime (triangle_mesh.py), which lib.py imports for a paper with meshes
MESH_RUNTIME_DIR = os.path.join(os.path.dirname(os.path.abspath(sys.modules['iheartla'].__file__)), 'mesh', 'Python')


def run_figure_script(source, cwd):
    """
    Run a figure's Python script in cwd
    :return: (succeeded, error message)
    """
    if not IN_BROWSER:
        env = dict(os.environ, PYTHONPATH=os.pathsep.join(filter(None, [MESH_RUNTIME_DIR, os.environ.get('PYTHONPATH')])))
        ret = subprocess.run([sys.executable, source], cwd=cwd, env=env, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        return ret.returncode == 0, ret.stderr.decode('UTF-8')
    # Run it in this interpreter, as a fresh __main__ that imports the current lib.py
    import runpy
    import traceback
    old_cwd = os.getcwd()
    os.chdir(cwd)
    sys.path.insert(0, cwd)
    sys.path.append(MESH_RUNTIME_DIR)
    sys.modules.pop('lib', None)
    try:
        runpy.run_path(source, run_name='__main__')
        return True, ''
    except SystemExit as e:
        if e.code in (None, 0):
            return True, ''
        return False, traceback.format_exc()
    except BaseException:
        return False, traceback.format_exc()
    finally:
        if 'matplotlib.pyplot' in sys.modules:
            sys.modules['matplotlib.pyplot'].close('all')
        sys.path.remove(cwd)
        sys.path.remove(MESH_RUNTIME_DIR)
        os.chdir(old_cwd)

def paper_requirements(content):
    """
    The PyPI requirements that the paper's YAML front matter lists for its figures, which the editor installs:
        requirements:
          - libigl>=2.6.4.dev3
    Outside the browser, figures run with this environment's Python, which must already have them.
    :return: a list of requirement strings, such as "libigl>=2.6.4.dev3"
    """
    import yaml
    lines = content.split('\n')
    if lines[0] != '---':
        return []
    meta_lines = []
    for line in lines[1:]:
        if line in ('---', '...'):
            break
        meta_lines.append(line)
    try:
        meta = yaml.safe_load('\n'.join(meta_lines))
    except yaml.YAMLError:
        # compiling the paper reports it
        return []
    requirements = meta.get('requirements') if isinstance(meta, dict) else None
    if isinstance(requirements, str):
        requirements = [requirements]
    if not isinstance(requirements, list):
        return []
    return [str(r).strip() for r in requirements if r is not None and str(r).strip()]

def update_resources(dst):
    """
    Copy extras/heartdown-resource to dst, replacing only the files that differ (left from an older H❤️rtDown)
    """
    def copy_if_changed(src, dst):
        if not (os.path.exists(dst) and filecmp.cmp(src, dst, shallow=False)):
            shutil.copy2(src, dst)
        return dst
    shutil.copytree(os.path.join(APP_DIR, "extras", "heartdown-resource"), dst,
                    copy_function=copy_if_changed, dirs_exist_ok=True)

def handle_title(text, dict):
    content = ''
    if "title" in dict:
        title = dict["title"]
        content += "<div class='title'>{}</div>".format(title)
    if "author" in dict:
        authors = dict["author"]
        for author in authors:
            content += "<div class='author'>{}, {}</div>".format(author['name'], author['affiliation'])
    content += text
    return content

def handle_abstract(text, dict):
    # abstract = text[:text.index('\n<h1')]
    # ABSTRACT_RE = re.compile(
    #     dedent(r'''\<p\>(?P<abstract>[^<>]*)\<\/p\>\n\<h1'''),
    #     re.MULTILINE | re.DOTALL | re.VERBOSE
    # )
    # # print("abstract:{}".format())
    # res_abstract = ''
    # for m in ABSTRACT_RE.finditer(text):
    #     abstract = m.group('abstract')
    #     res_abstract = "<p class='abstract'>{}</p>".format(abstract)
    #     text = text.replace("<p>{}</p>".format(abstract), '')
    #     # print("abstract:{}".format(m.group('abstract')))
    #     break
    res_abstract = ''
    if "abstract" in dict:
        abstract = dict['abstract']
        res_abstract = "<p class='abstract'>{}</p>".format(abstract)
        text = text.replace("<p>{}</p>".format(abstract), '')
    return text, res_abstract

def handle_sections(text, dict):
    map_dict = {}
    order_dict = {}
    def get_section_list(content, index=1, pre_str='', cur_base_list=[]):
        tag = ''
        sec_list = []
        title_list = []
        SECTION_RE = re.compile(
            dedent(r'''\<h{}\ id\=\"(?P<id>[^<>]*)\"\>(?P<title>[^<>]*)\<\/h{}\>\n'''.format(index, index)),
            re.DOTALL | re.VERBOSE
        )
        end_list = []
        cur_base = 1
        if len(cur_base_list) > 0:
            cur_base = cur_base_list.pop(0)
        cur_order = cur_base
        for m in SECTION_RE.finditer(content):
            # print("id:{}".format(m.group('id')))
            id_str = m.group('id')
            sec_list.append(id_str)
            if pre_str == '':
                title = "{}&nbsp;{}".format(cur_order, m.group('title'))
            else:
                title = "{}.{}&nbsp;{}".format(pre_str, cur_order, m.group('title'))
            title_list.append(title)
            order_dict[m.group()] = "<h{} id='{}'>{}</h{}>".format(index, id_str, title, index)
            cur_order += 1
            end_list.append(m.end())
        end_list.append(len(content))
        if len(sec_list) > 0:
            tag += "<ul>"
            pre_cur_index = cur_base
            for cur_index in range(len(sec_list)):
                tag += "<li><a href='#{}'>{}</a>".format(sec_list[cur_index], title_list[cur_index])
                cur_content = content[end_list[cur_index]:end_list[cur_index+1]]
                if pre_str == '':
                    new_pre = pre_cur_index
                else:
                    new_pre = "{}.{}".format(pre_str, pre_cur_index)
                cur_list, cur_tag = get_section_list(cur_content, index+1, new_pre, cur_base_list)
                if len(cur_list) > 0:
                    map_dict[sec_list[cur_index]] = cur_list
                    tag += cur_tag
                tag += "</li>"
                pre_cur_index += 1
            tag += "</ul>"
        return sec_list, tag
    section_base_list = []
    if 'sectionBase' in dict:
        section_base_list = dict['sectionBase']
    res_list, res_tag = get_section_list(text, 1, pre_str='', cur_base_list=section_base_list)
    # print("res_list:{}, map_dict:{}".format(res_list, map_dict))
    # print("res_tag:{}".format(res_tag))
    # text = "{}\n{}".format(res_tag, text)
    for k, v in order_dict.items():
        text = text.replace(k, v)
    return text

def handle_context_block(text):
    # Match string: ❤ : context
    CONTEXT_RE = re.compile(
        dedent(r'''<div>❤(\s*):(?P<context>[^\n❤]*)<\/div>'''),
        re.MULTILINE | re.VERBOSE
    )
    start_index = 0
    text_list = []
    context_list = ['']
    matched_list = ['']
    context_dict = {}
    for m in CONTEXT_RE.finditer(text):
        cur_context = m.group('context').strip()
        # print("parsed context: {}".format(m.group('context')))
        context_list.append(cur_context)
        matched_list.append(m.group())
        text_list.append(text[start_index: m.start()])
        start_index = m.end()
        if cur_context not in context_dict:
            context_dict[cur_context] = 0
    text_list.append(text[start_index:len(text)])
    for index in range(len(text_list)):
        cur_context = context_list[index]
        if cur_context != '':
            text_list[index] = "<div class='context' id='context-{}-{}' context='{}'>{}</div>".format(cur_context.replace(" ", ""), context_dict[cur_context], cur_context, text_list[index])
            context_dict[cur_context] += 1
    return ''.join(text_list)


def gen_figure(source, name, input_dir, figure_dict, figure_code, lib_changed):
    src = "{}/{}/lib.py".format(input_dir, OUTPUT_CODE)
    if os.path.exists(src):
        shutil.copyfile(src, "{}/{}/lib.py".format(input_dir, IMG_CODE))
    content = read_from_file(source)
    cur_hash = hashlib.md5("{}:{}".format(name, content).encode()).hexdigest()
    if not lib_changed:
        if cur_hash in cached_figure_code:
            figure_code[cur_hash] = cached_figure_code[cur_hash]
            if name in cached_figure_dict:
                figure_dict[name] = cached_figure_dict[name]
            return
    succeeded, err = run_figure_script(source, "{}/{}".format(input_dir, IMG_CODE))
    record("figure")
    if succeeded:
        figure_code[cur_hash] = True
    else:
        figure_code[cur_hash] = False
        figure_dict[name] = err
        print("failed, {}".format(source))


def handle_figure(text, name_list, input_dir, md):
    lib_changed = False
    for context, changed in md.changed_dict.items():
        if changed:
            lib_changed = True
            break
    start_index = 0
    text_list = []
    threads_list = []
    folder = "{}/{}".format(input_dir, IMG_CODE)
    figure_dict = {}  # figure name : err msg
    figure_code = {}  # figure name+content : True/False
    if not os.path.exists(folder):
        os.mkdir(folder)
    for m in FIGURE_BLOCK_RE.finditer(text):
        figure = m.group('figure')
        new_figure = None
        for img in IMAGE_BLOCK_RE.finditer(figure):
            src = img.group('src')
            path, name = get_file_base(src)
            suffix = get_file_suffix(src)
            # print("handle_figure, img: {}, name:{}".format(path, name))
            if name in name_list:
                source = "{}/{}.py".format(folder, name)
                threads_list.append(threading.Thread(target=gen_figure, args=(source, name, input_dir, figure_dict, figure_code, lib_changed)))
                pre_tag = """<pre class="errorMsg" hidden></pre>"""
                if suffix == 'html':
                    height = IMAGE_HEIGHT_RE.search(img.group('before') + ' ' + img.group('after'))
                    height = height.group('height') if height else FIGURE_HEIGHT
                    new_c = """<iframe id="{}" scrolling="no" style="border:none;" seamless="seamless" src="{}/{}.html" height="{}" width="100%"></iframe>""".format(name, path, name, height)
                    new_figure = "<figure{}>".format(m.group('property'))
                    new_figure += pre_tag + figure[:img.start()] + new_c + figure[img.end():]
                    new_figure += "</figure>"
                else:
                    new_figure = "<figure{}>".format(m.group('property'))
                    new_figure += pre_tag + figure
                    new_figure += "</figure>"
            break
        text_list.append(text[start_index: m.start()])
        start_index = m.end()
        if new_figure:
            text_list.append(new_figure)
        else:
            text_list.append(m.group())
    text_list.append(text[start_index:len(text)])
    if len(text_list) > 0:
        if len(threads_list) > 0:
            # print("generating figures ...")
            if IN_BROWSER:
                [t.run() for t in threads_list]
            else:
                [t.start() for t in threads_list]
                [t.join() for t in threads_list]
        record("handle_figure")
        global cached_figure_dict, cached_figure_code
        cached_figure_dict = copy.deepcopy(figure_dict)
        cached_figure_code = copy.deepcopy(figure_code)
        return ''.join(text_list), figure_dict
    return text, figure_dict


def save_output_code(md, path):
    # if not WHEEL_MODE:
    if True:
        dst = "{}/{}".format(path, OUTPUT_CODE)
        if os.path.exists(dst):
            shutil.rmtree(dst)
        os.mkdir(dst)
        if md.lib_py != '':
            save_to_file(md.lib_py, "{}/{}/lib.py".format(path, OUTPUT_CODE))
        if md.lib_cpp != '':
            save_to_file(md.lib_cpp, "{}/{}/lib.h".format(path, OUTPUT_CODE))
        if md.lib_matlab != '':
            save_to_file(md.lib_matlab, "{}/{}/lib.m".format(path, OUTPUT_CODE))


def process_input(content, input_dir='.', resource_dir='.', file_name='result',
                  parser_type=ParserTypeEnum.NUMPY | ParserTypeEnum.EIGEN | ParserTypeEnum.MATLAB,
                  server_mode=False):
    """
    Given the source string, generate the html result
    :param content: Markdown source
    :param input_dir: Path to Markdown file
    :param resource_dir: Path to resource
    :param file_name: Markdown file without extension
    :param parser_type: Output code
    :return: html content
    """
    record()
    if True:
    # try:
        extension_list = ['markdown.extensions.mdx_bib', \
                           'markdown.extensions.iheartla_code', \
                           'markdown.extensions.mdx_math', \
                           'markdown.extensions.attr_list', \
                           'markdown.extensions.fenced_code', \
                           'markdown.extensions.abbr', \
                           'markdown.extensions.def_list', \
                           'markdown.extensions.footnotes', \
                           'markdown.extensions.md_in_html', \
                           'markdown.extensions.tables', \
                           'markdown.extensions.admonition', \
                           # 'markdown.extensions.codehilite', \
                           'markdown.extensions.legacy_attrs', \
                           'markdown.extensions.legacy_em', \
                           'markdown.extensions.meta', \
                           'markdown.extensions.nl2br', \
                           'markdown.extensions.sane_lists', \
                           'markdown.extensions.smarty', \
                           'markdown.extensions.toc', \
                           'markdown.extensions.wikilinks']
        md = Markdown(extensions=extension_list,
                               path=input_dir,
                               parser_type=parser_type,
                               bibtex_file='{}/{}.bib'.format(input_dir, file_name))
        record("Call Markdown")
        body = md.convert(content)
        body, abstract = handle_abstract(body, md.Meta)
        body = handle_sections(body, md.Meta)
        body = abstract + body
        body = handle_title(body, md.Meta)
        body = handle_context_block(body)
        save_output_code(md, input_dir)
        figure_dict = {}
        if md.need_gen_figure:
            body, figure_dict = handle_figure(body, md.figure_list, input_dir, md)
        equation_json = md.json_data
        # equation_data = get_sym_data(json.loads(equation_json))
        sym_json = md.json_sym
        # where the page looks for it: resource_dir is relative to the paper
        dst = os.path.join(input_dir, resource_dir, "heartdown-resource")
        update_resources(dst)
        script = r"""window.onload = onLoad;
        function reportWindowSize() {
          var arrows = document.querySelectorAll(".arrow");
          if (arrows) {
            for (var i = arrows.length - 1; i >= 0; i--) {
              var arrow = arrows[i];
              var body = document.querySelector("body");
              var style = window.getComputedStyle(body);
              var arrowPanel = document.querySelector("#arrows");
              var arrowRect = arrowPanel.getBoundingClientRect();
              var curOffset = parseInt(style.marginLeft, 10)
              var oldOffset = arrow.getAttribute('offset');
              curOffset += arrowRect.width;
              arrow.setAttribute('offset', curOffset);
              // console.log(`oldOffset:${oldOffset}, curOffset:${curOffset}`);
              var arrowStyle = window.getComputedStyle(arrow); 
              var arrowOffset = parseInt(document.querySelector(".arrow").style.marginLeft, 10)
              arrow.style.marginLeft = `${arrowOffset+curOffset-oldOffset}px`;
              var newWidth = parseInt(style.width, 10) + parseInt(style.marginLeft, 10) + parseInt(style.marginRight, 10);
              arrow.style.width = `${newWidth}px`;
              arrow.style.height = style.height; 
              // console.log(`arrow.style.width:${arrow.style.width}, arrow.style.height:${arrow.style.height}`)
            }
          }
          adjustGlossaryBtn();
        }
        window.onresize = reportWindowSize;
        document.addEventListener("click", function(evt){
            onClickPage();
        });
        """
        mathjax = r'''<script>
        MathJax = {
          loader: {
            load: ["[attrLabel]/attr-label.js"],
            paths: { attrLabel: "''' + resource_dir + '''/heartdown-resource" },
          },
          tex: { packages: { "[+]": ["attr-label"] },
           inlineMath: [['$', '$']]
           },
           options: {
            enableAssistiveMml: false
          },
        };
            </script>'''
        html = r"""<html lang="en">
        <head>
            <meta charset="UTF-8">
            <meta http-equiv="X-UA-Compatible" content="IE=edge,chrome=1"> 
            {mathjax}
            <script type="text/javascript" id="MathJax-script" async src="https://cdn.jsdelivr.net/npm/mathjax@3/es5/tex-mml-chtml.js"></script>
            <script src="https://unpkg.com/@popperjs/core@2"></script>
            <script src="https://unpkg.com/tippy.js@6"></script>
            <link rel="stylesheet" href="https://unpkg.com/tippy.js@6/dist/svg-arrow.css"/>
            <link rel="stylesheet" href="https://unpkg.com/tippy.js@6/dist/border.css" />
            <script src="{resource_dir}/heartdown-resource/d3.min.js"></script>
            <script src="{resource_dir}/heartdown-resource/svg.min.js"></script>
            <script type="text/javascript" src='{resource_dir}/heartdown-resource/paper.js'></script>
            <link rel="stylesheet" href="{resource_dir}/heartdown-resource/paper.css">
        </head>
        <script>
        const iheartla_data = JSON.parse('{equation_json}');
        const sym_data = JSON.parse('{sym_json}');
        {script}
        </script>
<body>
    <div id="flexer">
        <div id="arrows"></div>
        <div id="main">
        {body}
        </div>
        <div id="glossary_holder"><div id="glossary" class="glossary"></div></div>
    </div>
</body>
</html>""".format(mathjax=mathjax, equation_json=equation_json, sym_json=sym_json, script=script, body=body,
                          resource_dir=resource_dir)
        # numbering
        EQ_BLOCK_RE = re.compile(
            dedent(r'''(?<!\\)((?<!\$)\${2}(?!\$))((?P<code>.*?))(?<!\\)(?<!\$)\1(?!\$)'''),
            re.MULTILINE | re.DOTALL | re.VERBOSE
        )
        num = md.Meta.get('eqBase', 1)
        for m in EQ_BLOCK_RE.finditer(html):
            equation = m.group('code')
            if '\\notag' not in equation:
                html = html.replace(m.group(), "$${}\\tag{{{}}}\\label{{{}}}$$".format(equation, num, num))
                num += 1
        ret = [html, md.lib_py, md.lib_cpp, md.lib_matlab]
        save_to_file(html, "{}/{}.html".format(input_dir, file_name))
    # except:
    #     ret = str(sys.exc_info()[0])
    # finally:
        return ret, figure_dict


def compile_paper(content, input_dir, file_name):
    """
    Compile a paper and report errors instead of raising them (for the editor)
    :return: dict with "ret" (0 on success), "res" ([html, lib_py, lib_cpp, lib_matlab]), "fig" (figure name: error),
             or "msg" and "expr" (the I❤️LA code with the error)
    """
    result = {"ret": 1, "name": file_name}
    figure_dict = {}
    try:
        res, figure_dict = process_input(content, input_dir, file_name=file_name, server_mode=True)
        result["ret"] = 0
        result["res"] = res
    except AssertionError as e:
        err_msg = LaMsg.getInstance()
        result["expr"] = err_msg.cur_code.replace('\n', '')
        result["msg"] = err_msg.cur_msg if err_msg.cur_msg else str(e)
    except Exception as e:
        result["msg"] = "{}: {}".format(type(e).__name__, str(e))
    if len(figure_dict) > 0:
        result["fig"] = figure_dict
    return result


def run_figure(name, source, input_dir):
    """
    Save and run the code of one figure, as edited in the editor, with the paper's current lib.py
    :return: (succeeded, error message)
    """
    folder = "{}/{}".format(input_dir, IMG_CODE)
    lib = "{}/{}/lib.py".format(input_dir, OUTPUT_CODE)
    if os.path.exists(lib):
        shutil.copyfile(lib, "{}/lib.py".format(folder))
    script = "{}/{}.py".format(folder, name)
    save_to_file(source, script)
    return run_figure_script(script, folder)


def main():
    arg_parser = argparse.ArgumentParser(description='HeartDown paper compiler')
    arg_parser.add_argument('--regenerate-grammar', action='store_true', help='Regenerate grammar files')
    arg_parser.add_argument('--resource_dir', help='resource path')
    arg_parser.add_argument('-o', '--output', help='The output language', choices=['numpy', 'eigen', 'latex','matlab'])
    arg_parser.add_argument('--paper', nargs='*', help='paper text')
    arg_parser.add_argument('-q', '--quiet', action='store_true', help='Hide warnings (such as undescribed symbols)')
    arg_parser.add_argument('--compiler', choices=['rust', 'python'],
                            help='The I❤️LA compiler (default: the HEARTDOWN_COMPILER environment variable, or rust)')
    # iheartla makes `import re` return `regex`, which breaks argparse's usage and error formatting
    iheartla_re = sys.modules['re']
    sys.modules['re'] = STDLIB_RE
    try:
        args = arg_parser.parse_args()
    finally:
        sys.modules['re'] = iheartla_re
    if args.compiler:
        os.environ['HEARTDOWN_COMPILER'] = args.compiler
    # LaLogger.getInstance().set_level(logging.DEBUG if DEBUG_MODE else logging.ERROR)
    LaLogger.getInstance().set_level(logging.ERROR if args.quiet else logging.WARNING)
    failed = []
    resource_dir = args.resource_dir if args.resource_dir else '.'
    # print("args.paper is {}, resource_dir is {}".format(args.paper, resource_dir))
    if args.regenerate_grammar:
        DEBUG_PARSER = True
        import iheartla.la_tools.parser_manager
        iheartla.la_tools.parser_manager.recreate_local_parser_cache()
    else:
        parser_type = ParserTypeEnumDefaults
        out_dict = {"numpy": ParserTypeEnum.NUMPY, "eigen": ParserTypeEnum.EIGEN, "latex": ParserTypeEnum.LATEX,
                    "mathjax": ParserTypeEnum.MATHJAX, "matlab": ParserTypeEnum.MATLAB}
        if args.output:
            # when output args are present _only_ output those
            parser_type = ParserTypeEnum.INVALID
            out_list = args.output.split(",")
            for out in out_list:
                assert out in out_dict, "Parameters after -o or --output can only be numpy, eigen, latex, or matlab"
                parser_type = parser_type | out_dict[out]
        else:
            parser_type = ParserTypeEnum.NUMPY | ParserTypeEnum.EIGEN | ParserTypeEnum.MATLAB
        for paper_file in args.paper or []:
            content = read_from_file(paper_file)
            base_name = os.path.basename(Path(paper_file))
            if DEBUG_MODE:
                ret, figure_dict = process_input(content, os.path.dirname(os.path.abspath(paper_file)), resource_dir, os.path.splitext(base_name)[0], parser_type)
            else:
                try:
                    ret, figure_dict = process_input(content, os.path.dirname(os.path.abspath(paper_file)), resource_dir, os.path.splitext(base_name)[0], parser_type)
                except AssertionError as e:
                    err_msg = LaMsg.getInstance()
                    print("{}: {}".format(paper_file, err_msg.cur_msg if err_msg.cur_msg else str(e)), file=sys.stderr)
                    print("expr: {}".format(err_msg.cur_code.replace('\n', '')), file=sys.stderr)
                    failed.append(paper_file)
                except Exception as e:
                    print("{}: {}: {}".format(paper_file, type(e).__name__, str(e)), file=sys.stderr)
                    failed.append(paper_file)
    record("End")
    if failed:
        sys.exit(1)


if __name__ == '__main__':
    main()
