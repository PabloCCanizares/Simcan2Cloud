#!/usr/bin/env python3.8
# *_* coding: utf-8 *_*

"""
module submit - Submits an execution to pbs queue.

This module submits jobs specified in a file to the PBS queue in GALGO
"""

__version__ = "1.0.0"
__author__ = "Hernán"  # only code writers
__email__ = "hernanindibil.cruz@uclm.es"
__maintainer__ = "Hernán"  # the person who will fix bugs and make improvements
# __copyright__ = "Copyright 2020, Simcan2Cloud"
# __license__ = "GPL"
__status__ = "Prototype"  # Prototype, Development or Production
__credits__ = ["Hernán I. de la Cruz", "Adrián Bernal"]

import asyncio
import os
import os.path
import sys

from job_utils import parse_job_file, Action


def main(argc, argv):
    """
    Check whether arguments are valid or not before calling launcher.

    Keyword arguments:
        argc -- the number of arguments, must be 2
        argv -- the list of arguments: the job file and the simulations folder
    """
    if (argc != 2):
        script = os.path.split(sys.argv[0])[1]
        print("Syntax: " + script + " <job file> <simulations path>",
              file=sys.stderr)
        sys.exit(1)
    job_file = argv[0]
    if not os.path.isfile(job_file):
        print("Job file doesn't exist!", file=sys.stderr)
        sys.exit(2)
    simulations_folder = argv[1]
    if not os.path.isdir(simulations_folder):
        print("Simulations folder doesn't exist!", file=sys.stderr)
        sys.exit(3)
    code = asyncio.run(launcher(job_file, simulations_folder))
    sys.exit(code)


async def launcher(file_name, simulations_path):
    """
    Call methods when needed to launch the simulations.

    Keyword arguments:
        file_name -- the name of the file to be parsed
        simulations_path -- the name of the folder containing the omnetpp.ini

    Returns:
        The error code (0 means no error)
    """
    code, jobs = fileparser(file_name)
    if code == 0:
        # simulations_path = "${HOME}/SIMCAN-2.0/simulations"
        tasks = set()
        for job in jobs:
            if job.action == Action.OMIT:
                print("omited:", job)
            else:
                print("running:", job)
                coroutine = job.launch(simulations_path, f"simcan_{job.cluster.value}.pbs")
                task = asyncio.create_task(coroutine)
                tasks.add(task)
        done, pending = await asyncio.wait(tasks)
        for task in done:
            code = task.result()
            if code != 0:
                print(f"Task failed with code {code}", file=sys.stderr)
                break

    return code


def fileparser(filename):
    """
    Parse the file and return the error code and the parsed list of jobs.

    Keyword arguments:
        filename -- the name of the file to be parsed

    Returns:
        The error code (0 means no error) and the list of jobs
    """
    code, jobs = parse_job_file(filename)
    if code != 0:
        print("Job line format: [omit|simulate|process|paint],x_coord,y_coord,"
              "labels_file,name,min_key,max_key")
    return (code, jobs)


if __name__ == "__main__":
    main(len(sys.argv) - 1, sys.argv[1:])
