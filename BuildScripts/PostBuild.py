import argparse, subprocess, json, os, shutil
from pathlib import Path

ignore_file_extensions = []

def compile_shaders(project_name : str, vulkan_sdk : str, shader_extensions : list[str], compiler : json, directories : json, script_dir : Path):
    shader_dir = directories["shaders"]
    content_dir = directories["content"]
    solution_dir = script_dir / directories["solution"]

    input_dir = Path(solution_dir) / Path(project_name) / Path(content_dir) / Path(shader_dir)
    collected_files = [
        file for file in input_dir.rglob("*")
        if file.is_file() and file.suffix.lower()[1:] in shader_extensions
    ]

    vulkan_bin = Path(os.path.expandvars(vulkan_sdk)) / 'Bin/'
    compiler_executable = vulkan_bin / Path(str(compiler["name"]) + '.exe')

    for file in collected_files:
        out_file = Path(input_dir) / Path(str(file.stem) + ".spv")

        file_contents = file.read_text(encoding="utf-8")
        # this is a little hacky, but given the way slang works...
        if file_contents.find('[shader("') == -1:
            print('[INFO] Skipping: ' + file.stem + file.suffix.lower() + ' due to not containing an entry-point!')
            continue

        print('[INFO] Compiling: ' + file.stem + file.suffix.lower() + ' -> ' + file.stem + '.spv')

        run = [
            str(compiler_executable)
        ]

        run.extend(compiler["flags"])
        run.extend(["-o", str(out_file),
            str(file)])

        subprocess.run(run)

def ignore_content(path, files):
    ignored = set()

    for file in files:
        if Path(file).suffix[1:].lower() in ignore_file_extensions:
            ignored.add(file)

    return ignored

def copy_content(project_name : str, out_dir : str, directories : json, script_dir : Path):
    content_dir = directories["content"]
    config_dir = directories["config"]
    dlls_dir = directories["dlls"]
    solution_dir = script_dir / directories["solution"]

    input_dir = Path(solution_dir) / Path(project_name) / Path(content_dir)
    output_dir = Path(out_dir) / Path(content_dir)
    
    shutil.copytree(input_dir, output_dir, dirs_exist_ok=True, ignore=ignore_content)

    input_dir = Path(solution_dir) / Path(project_name) / Path(config_dir)
    output_dir = Path(out_dir) / Path(config_dir)

    shutil.copytree(input_dir, output_dir, dirs_exist_ok=True, ignore=ignore_content)

    input_dir = Path(solution_dir) / Path(dlls_dir)
    output_dir = Path(out_dir)

    shutil.copytree(input_dir, output_dir, dirs_exist_ok=True, ignore=ignore_content)

parser = argparse.ArgumentParser()
parser.add_argument("output", help="The output folder for the copied.", type=str)
args = parser.parse_args()

script_dir = Path(__file__).resolve().parent
with open(str(script_dir) + '/options.json') as json_data:
    data = json.load(json_data)

    project = data["project_name"]
    vulkan_sdk = data["vulkan_sdk"]
    shader_ext = data["shader_extensions"]
    ignore_ext = data["ignore_extensions"]
    compiler = data["compiler"]
    directories = data["directories"]

    ignore_file_extensions.extend(ignore_ext)

    compile_shaders(project, vulkan_sdk, shader_ext, compiler, directories, Path(script_dir))
    copy_content(project, args.output, directories, Path(script_dir))