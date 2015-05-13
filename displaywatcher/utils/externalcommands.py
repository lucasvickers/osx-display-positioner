import subprocess
import logging
import os

def check_display_positions(settings=None):

    command = [ settings['displaypositioner_bin'] ]
    command += settings['displaypositioner_args']

    sp = subprocess.Popen(command,
                          stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    out, err = sp.communicate()

    fail = True
    try:
        translated_result = settings['displaypositioner_output_translation'][out.strip()]
    except Exception as e:
        logging.exception("Translating the displaypositioner output failed, check config + executable.")
        logging.exception(e)
        raise Exception("Unable to parse output from executable.")

    if sp.returncode:
        logging.exception("Displaypositioner returned a bad rcode, please check app.")
        raise Exception("Bad return code from external executable.")

    # return T/F based output
    return translated_result


def reboot_system(settings=None):
    os.system( settings['reboot_command'] )

