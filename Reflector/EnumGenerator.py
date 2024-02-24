import os
import re

enum_pattern = re.compile(r'enum\s+class\s+(\w+)\s*:\s*\w+\s*{([^}]+)};', re.MULTILINE | re.DOTALL)


def find_enums_in_file(file_path):
    with open(file_path, 'r') as file:
        content = file.read()
    enums = enum_pattern.findall(content)
    return [(enum_name, [value.strip() for value in values.split(',') if value.strip()]) for enum_name, values in enums]


def generate_enum_mappings(enums, output_dir):
    for enum_name, values in enums:
        output_path = os.path.join(output_dir, f"{enum_name}_EnumMappings.cpp")
        with open(output_path, 'w') as file:
            mappings = ',\n'.join(
                [f'{{ {enum_name}::{value.split("=")[0].strip()}, "{value.split("=")[0].strip()}" }}' for value in
                 values])
            file.write(f"""#include <unordered_map>
#include <string>
#include "{enum_name}.h"

static const std::unordered_map<{enum_name}, std::string> {enum_name}ToStringMap = {{
    {mappings}
}};

static const std::string& ToString({enum_name} value) {{
    static const std::string unknown = "Unknown";
    auto it = {enum_name}ToStringMap.find(value);
    if (it != {enum_name}ToStringMap.end())
        return it->second;
    return unknown;
}}
""")


def traverse_and_generate_mappings(src_dir, output_dir):
    for root, dirs, files in os.walk(src_dir):
        for file in files:
            if file.endswith('.h') or file.endswith('.cpp'):
                file_path = os.path.join(root, file)
                enums = find_enums_in_file(file_path)
                if enums:
                    generate_enum_mappings(enums, output_dir)


if __name__ == "__main__":
    src_dir = "../"
    output_dir = "../Meta/"
    traverse_and_generate_mappings(src_dir, output_dir)
