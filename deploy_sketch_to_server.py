#!/bin/python3

import sys
import getopt
import re
from os import getenv
from os import path
from os import mkdir
from os import system
from os import walk

from shutil import which
from shutil import copy2

FIRMWARE_HEADER = '#include <DeviceRuntime.h>'
FIRMWARE_VERSION_REGEX = 'VERSION \= \"([0-9]{1,2}\.[0-9]{1,3}\.[0-9]{1,4})\"\;'
DEVICE_ID_REGEX = 'DEVICE_ID \= \"([a-zA-Z0-9]{6})\"\;'
BUILD_NUMBER_REGEX = '(BUILD_NUMBER \=) ([0-9]{1,4})\;'


def get_regex_group_from_sketch(regex, sketch_data, group_number=1):
    result = re.search(regex, sketch_data)
    if result:
        return result.group(group_number)
    return None


def write_build_number_to_sketch(sketch_file_path, build_number):
    with open(sketch_file_path) as sketch_file_read:
        sketch_data = sketch_file_read.read()
        new_sketch_data = re.sub(BUILD_NUMBER_REGEX, str(
            "\\1 " + str(build_number) + ";"), sketch_data)
    with open(sketch_file_path, "w") as sketch_file_write:
        sketch_file_write.write(new_sketch_data)


def get_build_number_from_sketch(sketch_data):
    build_number_result = get_regex_group_from_sketch(
        BUILD_NUMBER_REGEX, sketch_data, 2)
    if build_number_result:
        return int(build_number_result)
    else:
        print("build number not found in sketch")
        exit(1)


def get_device_id_from_sketch(sketch_data):
    device_id_result = get_regex_group_from_sketch(
        DEVICE_ID_REGEX, sketch_data)
    if device_id_result:
        return device_id_result
    else:
        print("device id not found in sketch")
        exit(1)


def check_for_firmware_header(sketch_data):
    if sketch_data.startswith(FIRMWARE_HEADER):
        return True
    return False


def get_device_info_from_sketch(sketch_file_path):
    with open(sketch_file_path, 'r') as sketch_file:
        sketch_data = sketch_file.read()
        if check_for_firmware_header(sketch_data):
            device_id = get_device_id_from_sketch(sketch_data)
            build_number = get_build_number_from_sketch(sketch_data)
            return device_id, build_number
        else:
            print("sketch not compatible: " + sketch_file_path)
            exit(1)


def get_firmware_version_from_version_header(header_data):
    firmware_version_result = get_regex_group_from_sketch(
        FIRMWARE_VERSION_REGEX, header_data, 1)
    if firmware_version_result:
        return firmware_version_result
    else:
        print("firmware version not found in firmware header file")
        exit(1)


def build_sketch_file(sketch_file_path):
    device_id, build_number = get_device_info_from_sketch(sketch_file_path)
    next_build_number = build_number + 1
    print("Building '" + str(sketch_file_path) + "' with device id '" + device_id + "' in version '" +
          FIRMWARE_VERSION + "-" + str(next_build_number) + "'")
    write_build_number_to_sketch(sketch_file_path, next_build_number)
    build_command_line_call = "arduino-cli compile -b " + board_configuration + " "  \
        + sketch_file_path + " -e"
    print("build command line call: " + build_command_line_call)
    system(build_command_line_call)
    build_file_path = path.join(path.dirname(sketch_file_path),
                                "build",
                                board_configuration.replace(":", "."),
                                path.basename(sketch_file_path) + ".bin")
    print("sketch was successfully build at: " + build_file_path)
    return next_build_number, build_file_path


def publish_device_binaries(sketch_file_path, build_file_path, build_number):
    device_id, _ = get_device_info_from_sketch(sketch_file_path)
    if path.exists(build_file_path):
        device_update_folder = path.join(ESP_DEVICE_UPDATES_PATH, device_id)
        if not path.exists(device_update_folder):
            mkdir(device_update_folder)
        version_file_name = FIRMWARE_VERSION + \
            "-" + str(build_number) + ".bin"
        copy_path = path.join(device_update_folder, version_file_name)
        print("deploying to server at: " + copy_path)
        copy2(build_file_path, copy_path)
    else:
        print("build sketch could not be found after build")
        exit(1)


def find_all_sketches_in(directory):
    sketch_file_paths = list()
    for root, _, files in walk(directory):
        for file in files:
            if file.endswith(".ino"):
                file_path = path.join(root, file)
                with open(file_path, 'r') as sketch_file:
                    sketch_data = sketch_file.read()
                    if check_for_firmware_header(sketch_data):
                        sketch_file_paths.append(file_path)
    return sketch_file_paths


def usage():
    print("usage: deploy_sketch_to_server.py -s <sketch_file>")


# script start
print("checking arduino-cli")
if not which("arduino-cli"):
    print("arduino-cli not installed!")

print("checking ESP_DEVICE_UPDATES_PATH")
# init of the update environment
ESP_DEVICE_UPDATES_PATH = getenv('ESP_DEVICE_UPDATES_PATH')
if not path.exists(ESP_DEVICE_UPDATES_PATH):
    print("device update deployment folder not found")
    exit(1)

print("checking ESP_FIRMWARE_DIR")
ESP_FIRMWARE_DIR = getenv('ESP_FIRMWARE_DIR')
if not path.exists(ESP_FIRMWARE_DIR):
    print("esp firmware folder not found")
    exit(1)
else:
    print("checking version_header_path")
    version_header_path = path.join(ESP_FIRMWARE_DIR, "VERSION.h")
    if not path.exists(version_header_path):
        print("version header not found in firmware dir")
        exit(1)

with open(version_header_path, 'r') as version_header_file:
    print("checking FIRMWARE_VERSION")
    FIRMWARE_VERSION = get_firmware_version_from_version_header(
        version_header_file.read())

# checking of input args
try:
    if len(sys.argv) > 1:
        opts, args = getopt.getopt(sys.argv[1:], "s:b:l:a:")
    else:
        usage()
        sys.exit(2)
except getopt.GetoptError as err:
    usage()
    print(err)
    sys.exit(2)

sketch_file_path = None
board_configuration = "esp8266:esp8266:nodemcuv2"
build_all_in_folder = False
build_root_folder_path = None
for opt, arg in opts:
    if opt == "-s":
        sketch_file_path = arg
    elif opt == '-a':
        build_root_folder_path = arg
        build_all_in_folder = True
    elif opt == '-b':
        board_configuration = arg
    elif opt == "-h":
        usage()
        exit(2)
    elif opt == "-l":
        print("installed esp8266 boards: ")
        system("arduino-cli board listall | grep esp8266")
        exit(2)

if build_all_in_folder:
    if path.exists(build_root_folder_path):
        print("building all sketches in: " + build_root_folder_path)
        for sketch_file in find_all_sketches_in(build_root_folder_path):
            build_number, build_file_path = build_sketch_file(sketch_file)
            publish_device_binaries(
                sketch_file, build_file_path, build_number)
    else:
        print("folder does not exist: " + build_root_folder_path)
        exit(1)
else:
    if not path.exists(sketch_file_path):
        print("sketch does not exist: " + sketch_file_path)
        exit(1)

    # Building
    build_number, build_file_path = build_sketch_file(sketch_file_path)
    # Publishing
    publish_device_binaries(sketch_file_path, build_file_path, build_number)
