# Regex to DFA Simulator

A lightweight interactive tool that converts regular expressions into Deterministic Finite Automata (DFA), visualizes state transitions, and tests input strings.

## Features

- Convert regular expressions into DFA.
- Generate DFA transition tables.
- Visualize automata using Graphviz.
- Test strings for acceptance or rejection.
- Interactive browser-based interface.

## Supported Syntax

| Symbol | Operation |
|---|---|
| `+` | Union |
| `.` or implicit | Concatenation |
| `*` | Kleene Star |
| `()` | Grouping |

## Tech Stack

- **C++** – Automata processing
- **Python** – Backend integration
- **Streamlit** – User interface
- **Pandas** – Transition tables
- **Graphviz** – DFA visualization

## Project Structure

```text
├── cpp_backend/
│   ├── main.cpp
│   └── main.exe
├── python_frontend/
│   └── app.py
├── outputs/
│   └── dfa.dot
├── .gitignore
├── requirements.txt
├── run.bat
└── README.md
```

## Getting Started

### Requirements

- Python 3
- g++ compiler
- Graphviz

Ensure the required tools are added to your system PATH.

### Run

Clone the repository:

```bash
git clone <YOUR_REPOSITORY_URL>
```

Open the project directory in VS Code and execute:

```powershell
.\run.bat
```

Open the application at:

```text
http://localhost:8501
```

To stop the application, press `Ctrl + C` in the terminal.

## How It Works

1. Enter a regular expression.
2. The expression is validated and converted into postfix notation.
3. An intermediate ε-NFA is constructed.
4. Subset construction converts it into a DFA.
5. View the resulting transition table and diagram.
6. Enter strings to check whether they are accepted.

The ε-NFA is used internally and is not displayed in the interface.

---

