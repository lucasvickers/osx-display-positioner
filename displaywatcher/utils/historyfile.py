import os
import logging

def get_num_previous_reboots(settings=None):
    if os.path.exists(settings['reboot_history_file']):
        try:
            with open(settings['reboot_history_file'], 'r') as infile:
                restarts_str = infile.read()
                restarts_int = int(restarts_str.strip())
                return restarts_int
        except Exception as e:
            logging.exception("Unable to read displaywatcher history file: " + HISTORY_FILE)
            logging.exception(e)
            raise Exception("Unable to read and parse the existing history file: " + HISTORY_FILE)

    # otherwise the file didn't exist, so we assume 0
    return 0


def reset_history_file(settings=None):
    set_history_file_count(settings=settings, count=0)


def set_history_file_count(settings=None, count=None):
    try:
        with open(settings['reboot_history_file'], 'w') as outfile:
            if count is None:
                raise Exception("count can not be None")

            outfile.write(str(count))
    except Exception as e:
        logging.exception("Unable to write displaywatcher history file: " + HISTORY_FILE)
        logging.exception(e)
        raise Exception("Unable to write to the history file: " + HISTORY_FILE)
