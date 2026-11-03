def pytest_addoption(parser):
    parser.addoption("--update-expected", action="store_true",
                     help="Save the compiler's output as the expected output of tests/corpus")
    parser.addoption("--browser", action="store_true",
                     help="Also run test_browser.py: the editor's demos in headless Google Chrome (needs the browser group)")
    parser.addoption("--compilers", action="store_true",
                     help="Also run test_compilers.py: compile each paper with the Rust and the Python I❤️LA compilers")
