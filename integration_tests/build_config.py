try:
    from config import config
except ImportError as e:
    raise ImportError(
        "Error loading config: %s. Did you forget to add <build>/integration_tests to PYTHONPATH?" % e.message)

