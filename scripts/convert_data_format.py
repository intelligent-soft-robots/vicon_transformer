#!/usr/bin/env python3
"""Convert a recorded pickle file of an older data format to the newest one.

The structure in which the Vicon data is provided changed in the past.  This script
converts files recorded with an older format to the newest one, so they can be used with
the latest version of the software.
"""
import argparse
import json
import logging
import pathlib
import pickle
import sys
import types
import typing


def convert_record_1to2(
    record: typing.Dict[str, typing.Any]
) -> typing.Dict[str, typing.Any]:
    del record["num_subjects"]
    del record["subjectNames"]

    subjects = {}
    for key in list(record.keys()):
        if key.startswith("subject_"):
            subject = record[key]
            name = subject["name"]

            # delete obsolete stuff
            del subject["name"]
            del subject["global_rotation"]["eulerxyz"]
            del subject["global_rotation"]["helical"]
            del subject["global_rotation"]["matrix"]

            subjects[name] = subject

            # remove the subject_# entry
            del record[key]

    record["subjects"] = subjects
    record["format_version"] = 2

    return record


def main() -> int:
    logging.basicConfig(level=logging.INFO)

    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "input_file", metavar="FILE", type=pathlib.Path, help="Input file"
    )
    parser.add_argument(
        "output_file", metavar="FILE", type=pathlib.Path, help="Output file."
    )
    args = parser.parse_args()

    loader: types.ModuleType
    if args.input_file.suffix == ".json":
        single_record = True
        loader = json
    else:
        single_record = False
        loader = pickle

    try:
        with open(args.input_file, "rb") as f:
            data = loader.load(f)
    except FileNotFoundError:
        logging.fatal("File %s does not exists.", args.input_file)
        return 1
    except Exception as e:
        logging.fatal("Failed to read file %s.  Error: %s", args.input_file, e)
        return 1

    # if only a single record is loaded, simply wrap it in a list for processing
    if single_record:
        data = [data]

    for record in data:
        if "format_version" not in record:
            record = convert_record_1to2(record)
        elif record["format_version"] == 2:
            logging.info("File is already in the latest format.")
        else:
            logging.fatal("Unexpected format version %s", record["format_version"])
            return 1

    # unwrap
    if single_record:
        data = data[0]

    if args.output_file.suffix == ".json":
        with open(args.output_file, "w") as f:
            json.dump(data, f, indent=4)
    else:
        with open(args.output_file, "wb") as f:
            pickle.dump(data, f, pickle.HIGHEST_PROTOCOL)

    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except KeyboardInterrupt:
        logging.info("You pressed Ctrl+C -> abort conversion")
        sys.exit(2)
