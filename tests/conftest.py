def pytest_addoption(parser):
    parser.addoption("--update-expected", action="store_true",
                     help="Save the compiler's output as the expected output of tests/corpus")
