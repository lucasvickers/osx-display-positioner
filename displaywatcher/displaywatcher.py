#!/usr/bin/env python

import yaml
import os
import logging
import logging.config
import sys

from utils.emailtools import notify_of_reboot, notify_of_max_reboots, notify_of_error
from utils.historyfile import get_num_previous_reboots, reset_history_file, set_history_file_count
from utils.externalcommands import check_display_positions, reboot_system

LOGGING_CONFIG_FILE = "logging.conf"
CONFIG_FILE = "config.yaml"


def check_monitors():

    try:
        logging.config.fileConfig(LOGGING_CONFIG_FILE)
    except Exception as e:
        print "Error loading logging config file: ", e
        return False

    try:
        settings = yaml.load(open(CONFIG_FILE, 'r'))
    except Exception as e:
        logging.exception("Error loading settings file.")
        logging.exception(e)
        return False

    try:

        positions_correct = check_display_positions(settings=settings)
        previous_reboots = get_num_previous_reboots(settings=settings)

        if positions_correct:

            logging.info("Display positions were correct, exiting normally.")
            reset_history_file(settings=settings)
            return True

        if previous_reboots < settings['max_reboots']:

            logStr = "Display positions incorrect, previously rebooted " + str(previous_reboots)
            logStr += " times of max " + str(settings['max_reboots']) + ", restarting system."
            logging.info(logStr)

            set_history_file_count(settings=settings, count=previous_reboots + 1)
            notify_of_reboot(settings=settings)
            reboot_system(settings=settings)
            return True

        # if we got here we have an error and we rebooted too many times
        logStr = "Rebooted " + str(previous_reboots) + " times without luck, giving up."
        logging.warn(logStr)

        reset_history_file(settings=settings)
        notify_of_max_reboots(settings=settings)
        return True


    except Exception as e:
        logging.exception("Error in main logic.")
        logging.exception(e)
        notify_of_error(settings=settings, message="Error in displaywatcher run, check logs.")
        return False


if __name__ == "__main__":

    # set our dir to local so we can grab our config files
    os.chdir(sys.path[0])

    if check_monitors():
        # clean run
        sys.exit(0)
    else:
        # bad run
        sys.exit(1)