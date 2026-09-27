# Contributing to Linux Manager

Thank you for your interest in contributing to Linux Manager!

---

## Code of Conduct

Please treat all community members with respect and kindness.

---

## Development Setup

1. **Clone the Repository**:
   ```bash
   git clone https://github.com/linuxmanager/linuxmanager.git
   cd linuxmanager
   ```

2. **Install Dependencies**:
   ```bash
   sudo apt update
   sudo apt install -y build-essential cmake pkg-config \
       qt6-base-dev qt6-declarative-dev qml6-module-qtquick \
       qml6-module-qtquick-controls qml6-module-qtquick-layouts \
       libsystemd-dev libdbus-1-dev dpkg-dev
   ```

3. **Build and Test**:
   ```bash
   make
   make test
   ```

---

## Architectural Guidelines

1. **C++20 Standard**: Use modern C++20 features (`std::filesystem`, `<ranges>`, `<concepts>`, RAII, const-correctness).
2. **Security First**:
   - Never introduce raw command execution functions with root privileges.
   - All privileged actions must be defined in `PrivilegedOperation.h` and dispatched through `PrivilegedDispatcher`.
   - Never delete user files outside approved user caches or recognized project build artifacts.
   - Always check symlinks and prevent directory traversal.
3. **Dual-Backend Testing**:
   - Write unit tests for new features using `MockSystemBackend`.
   - Never write tests that perform real destructive root actions on the host.

---

## Pull Request Workflow

1. Create a descriptive feature branch (`git checkout -b feature/my-cleaner`).
2. Implement your changes and add corresponding unit tests in `tests/test_main.cpp`.
3. Ensure all tests pass: `make test`.
4. Submit a Pull Request targeting `main`.
