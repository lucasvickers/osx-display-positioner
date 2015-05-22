import smtplib
from email.mime.text import MIMEText
from datetime import datetime
import email.utils
import socket
import logging


def notify_of_reboot(settings=None):

    body_text = "At " + datetime.now().isoformat() + "\nHost " + socket.gethostname()
    body_text += "\nWas rebooted in an attempt to fix display arrangement issues."

    msg = MIMEText(body_text)
    msg['Subject'] = "Notification of reboot on " + socket.gethostname()

    send_email(settings=settings, msg=msg)


def notify_of_max_reboots(settings=None, num_boots=None):

    body_text = "At " + datetime.now().isoformat() + "\nHost " + socket.gethostname()
    body_text += "\nAttempted reboots " + str(num_boots) + " times, and has given up."

    msg = MIMEText(body_text)
    msg['Subject'] = "Notification of max reboots on " + socket.gethostname()

    send_email(settings=settings, msg=msg)


def notify_of_error(settings=None, message=None):

    body_text = "At " + datetime.now().isoformat() + "\nHost " + socket.gethostname()
    body_text += "\nError Message: " + message

    msg = MIMEText(body_text)
    msg['Subject'] = "Notification message on " + socket.gethostname()

    send_email(settings=settings, msg=msg)


def send_email(settings=None, msg=None):

    try:
        msg['To'] = email.utils.formataddr((settings['receiving_name'], settings['receiving_email']))
        msg['From'] = email.utils.formataddr((settings['sending_name'], settings['sending_email']))

        server = smtplib.SMTP(settings['smtp_server'], settings['smtp_port'])

        # show communication with the server
        server.set_debuglevel(settings['smtp_debug_trace'])

        server.ehlo()

        if settings['smtp_encryption'].lower() == 'tls' and server.has_extn('STARTTLS'):
            server.starttls()
            server.ehlo()

        server.login(settings['smtp_username'], settings['smtp_password'])

        try:
            server.sendmail(settings['sending_email'], settings['receiving_email'], msg.as_string())
        finally:
            server.quit()

    except Exception as e:
        logging.exception("Error sending email.")
        logging.exception(e)

