import subprocess
from pathlib import Path

import pandas as pd
import streamlit as st

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "cpp_backend" / "main.cpp"
EXE = ROOT / "cpp_backend" / "main.exe"
OUT = ROOT / "outputs"
OUT.mkdir(exist_ok=True)

st.set_page_config(page_title="Regular Expression to DFA", page_icon="🔁", layout="wide")
st.title("Regular Expression to DFA Simulator")
st.caption("A simple Theory of Computation project")

with st.sidebar:
    st.header("Supported notation")
    st.markdown(
        "- `+` : Union (OR)\n"
        "- Concatenation: `ab`\n"
        "- `*` : Kleene star (zero or more)\n"
        "- `()` : Grouping"
    )
    st.caption("Operands: single letters, digits or underscore. A dot may optionally be used for explicit concatenation.")

regex = st.text_input("Regular expression", "a(b+c)*", help="Example: a(b+c)*")
test_string = st.text_input("String to test", "abcb", help="Enter the string to check. Leave empty to test the empty string.")


def run_backend():
    # Compile only when the executable is missing or the C++ source changed.
    if not EXE.exists() or SRC.stat().st_mtime > EXE.stat().st_mtime:
        result = subprocess.run(
            ["g++", "-std=c++17", str(SRC), "-o", str(EXE)],
            cwd=ROOT, capture_output=True, text=True
        )
        if result.returncode != 0:
            raise RuntimeError(result.stderr or "C++ compilation failed.")

    result = subprocess.run(
        [str(EXE)],
        input=regex + "\n" + test_string + "\n",
        cwd=ROOT, capture_output=True, text=True
    )
    if result.returncode != 0:
        message = result.stdout.strip() or result.stderr.strip()
        if message.startswith("ERROR="):
            message = message[len("ERROR="):]
        raise RuntimeError(message or "The simulator could not process this input.")
    return result.stdout


def parse_output(output):
    data = {"rows": [], "accepting": set()}
    for line in output.splitlines():
        if "=" not in line:
            continue
        key, value = line.split("=", 1)
        if key == "POSTFIX":
            data["postfix"] = value
        elif key == "DFA_ROW":
            parts = value.split(",")
            row = {"State": "q" + parts[0]}
            for item in parts[1:]:
                if ":" in item:
                    symbol, destination = item.split(":", 1)
                    row[symbol] = "—" if destination == "-" else "q" + destination
            data["rows"].append(row)
        elif key == "DFA_ACCEPT":
            data["accepting"] = set() if value == "-" else set(value.split(","))
        elif key == "TRACE":
            data["trace"] = value
        elif key == "RESULT":
            data["result"] = value
    return data


if st.button("Generate DFA", type="primary", use_container_width=True):
    try:
        st.session_state["result_data"] = parse_output(run_backend())
        st.session_state["last_regex"] = regex
        st.session_state["last_test"] = test_string
        st.session_state.pop("run_error", None)
    except Exception as error:
        st.session_state["run_error"] = str(error)

if st.session_state.get("run_error"):
    st.error(st.session_state["run_error"])

if "result_data" in st.session_state:
    data = st.session_state["result_data"]
    st.success("DFA generated for: " + st.session_state["last_regex"])

    tab_dfa, tab_simulation = st.tabs(["DFA", "String simulation"])
    with tab_dfa:
        st.subheader("DFA transition table")
        rows = []
        for original in data["rows"]:
            row = dict(original)
            state_number = row["State"][1:]
            row["State"] = ("→ " if state_number == "0" else "") + ("* " if state_number in data["accepting"] else "") + "q" + state_number
            rows.append(row)
        st.dataframe(pd.DataFrame(rows), use_container_width=True, hide_index=True)
        st.caption("→ = initial state; * = accepting state; — = no transition (rejecting dead condition).")

        st.subheader("DFA state diagram")
        diagram = OUT / "dfa.dot"
        if diagram.exists():
            st.graphviz_chart(diagram.read_text(encoding="utf-8"))
        else:
            st.info("DFA diagram is not available yet.")

    with tab_simulation:
        st.subheader("Input string")
        st.code(st.session_state["last_test"] if st.session_state["last_test"] else "λ (empty string)", language=None)
        st.write("State trace:", data.get("trace", ""))
        if data.get("result") == "ACCEPTED":
            st.success("ACCEPTED — the input string belongs to the language.")
        else:
            st.error("REJECTED — the input string does not belong to the language.")

st.divider()
st.caption("The program constructs an intermediate NFA internally, converts it into a DFA, and displays only the DFA and string-testing result.")
