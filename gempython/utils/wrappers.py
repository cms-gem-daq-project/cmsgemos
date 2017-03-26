
def runCommand(cmd):
    """
    Provides a wrapper around the subprocess.call command
    cmd should be an array of the form: ["command","arg1",...]
    """

    import datetime,os,sys
    import subprocess
    from subprocess import CalledProcessError
    try:
        print "Executing command",cmd
        sys.stdout.flush()
        returncode = subprocess.call(cmd)
    except CalledProcessError as e:
        print "Caught exception",e,"running",cmd
        sys.stdout.flush()
        pass
    return returncode
