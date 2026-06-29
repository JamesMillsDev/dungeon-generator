import argparse, json, Shaders, Resources
from pathlib import Path

parser = argparse.ArgumentParser()
parser.add_argument("output", help="The output folder for the copied.", type=str)
args = parser.parse_args()

script_dir : Path = Path(__file__).resolve().parent
with open(str(script_dir) + '/options.json') as json_data:
    data = json.load(json_data)

    project = data["project_name"]
    vulkan_sdk = data["vulkan_sdk"]
    shader_ext = data["shader_extensions"]
    ignore_ext = data["ignore_extensions"]
    compiler = data["compiler"]
    directories = data["directories"]

    Resources.ignore_file_extensions.extend(ignore_ext)

    Shaders.compile(project, vulkan_sdk, shader_ext, compiler, directories, script_dir)
    Resources.pack(project, directories, script_dir, "Resources")
    Resources.copy(project, args.output, "Resources", directories, script_dir)