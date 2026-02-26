from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CATALOG = ROOT / "docs" / "api_reference_catalog.json"
DOC_OUT = ROOT / "docs" / "API_REFERENCE.md"
CLI_HELP_OUT = ROOT / "tools" / "geotools_cli_help.inc"


def _esc_c(s: str) -> str:
    return s.replace('\\', '\\\\').replace('"', '\\"')


def build() -> None:
    data = json.loads(CATALOG.read_text())

    md: list[str] = []
    md.append("# geotools Unified API Reference")
    md.append("")
    md.append("This file is generated from `docs/api_reference_catalog.json` (single source of truth).")
    md.append("")
    md.append("## Data Types / Objects")
    for t in data["data_types"]:
        md.append(f"- **{t['name']}**: {t['description']}")
    md.append("")

    md.append("## C API Functions")
    for f in data["c_functions"]:
        md.append(f"### `{f['name']}`")
        md.append(f"- **What it does**: {f['description']}")
        md.append(f"- **Returns**: {f['returns']}")
        md.append(f"- **C syntax**: `{f['c_signature']}`")
        md.append(f"- **Python syntax**: `{f['python_wrapper']}`")
        md.append(f"- **C++ syntax**: `{f['cpp_wrapper']}`")
        md.append("- **Example (Python)**:")
        md.append("```python")
        md.append(f["example_python"])
        md.append("```")
        md.append("- **Example (C++)**:")
        md.append("```cpp")
        md.append(f["example_cpp"])
        md.append("```")
        md.append("")

    md.append("## Python API (public callables)")
    for p in data["python_api"]:
        md.append(f"- `{p['signature']}`")
    md.append("")

    md.append("## C++ Wrapper API")
    for c in data["cpp_api"]:
        md.append(f"- `{c['class']}::{c['member']}`")
    md.append("")

    md.append("## CLI Command Reference")
    for c in data["cli_commands"]:
        md.append(f"### `{c['name']}`")
        md.append(f"- **What it does**: {c['description']}")
        md.append(f"- **Usage**: `{c['usage']}`")
        md.append(f"- **Maps to**: {', '.join('`'+m+'`' for m in c['maps_to'])}")
        md.append("- **Example**:")
        md.append("```bash")
        md.append(c["example"])
        md.append("```")
        md.append("")

    DOC_OUT.write_text("\n".join(md) + "\n")

    lines: list[str] = []
    lines.append("/* Auto-generated from docs/api_reference_catalog.json */")
    lines.append("static const char* GEO_CLI_HELP_OVERVIEW =")
    lines.append('    "geotools-cli commands:\\n"')
    for c in data["cli_commands"]:
        lines.append(f'    "  { _esc_c(c["usage"]) }\\n"')
    lines.append('    ;')
    lines.append("")
    lines.append("static const char* geo_cli_help_for(const char* cmd) {")
    for i, c in enumerate(data["cli_commands"]):
        kw = "if" if i == 0 else "else if"
        lines.append(f'  {kw} (strcmp(cmd, "{_esc_c(c["name"])}") == 0) {{')
        lines.append("    return")
        lines.append(f'      "{_esc_c(c["name"])}\\n"')
        lines.append(f'      "  Description: {_esc_c(c["description"])}\\n"')
        lines.append(f'      "  Usage: {_esc_c(c["usage"])}\\n"')
        lines.append(f'      "  C API: {_esc_c(", ".join(c["maps_to"]))}\\n"')
        lines.append(f'      "  Example: {_esc_c(c["example"])}\\n";')
        lines.append("  }")
    lines.append('  return "Unknown command for help. Use: geotools-cli help\\n";')
    lines.append("}")
    CLI_HELP_OUT.write_text("\n".join(lines) + "\n")


if __name__ == "__main__":
    build()
