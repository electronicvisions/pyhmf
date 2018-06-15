import sys
import subprocess

def get_width():
    process = subprocess.Popen(['tput', 'cols'], stdout=subprocess.PIPE)
    return int(process.communicate()[0])


def writeln(left, right="", indentation=0):
    if not isinstance(left, CString):
        left = CString(left)
    if not isinstance(right, CString):
        right = CString(right)
    sys.stdout.write(indentation*" " + str(left) + (get_width() - indentation - len(left) - len(right))*" " + str(right) + '\n')


SEQUENCE = '\033[{0};{1};{2}m'

FORMATTING = {
    'default':      0,
    'bold':         1,
    'dim':          2,
    'underlined':   4,
    'blink':        5,
    'reverse':      7,
    'hidden':       8
}

FOREGROUND = {
    'default':      39,
    'black':        30,
    'red':          31,
    'green':        32,
    'yellow':       33,
    'blue':         34,
    'magenta':      35,
    'cyan':         36,
    'lightgrey':    37,
    'darkgrey':     90,
    'lightred':     91,
    'lightgreen':   92,
    'lightyellow':  93,
    'lightblue':    94,
    'lightmagenta': 95,
    'lightcyan':    96,
    'white':        97
}

BACKGROUND = {
    'default':      49,
    'black':        40,
    'red':          41,
    'green':        42,
    'yellow':       43,
    'blue':         44,
    'magenta':      45,
    'cyan':         46,
    'lightgrey':    47,
    'darkgrey':     100,
    'lightred':     101,
    'lightgreen':   102,
    'lightyellow':  103,
    'lightblue':    104,
    'lightmagenta': 105,
    'lightcyan':    106,
    'white':        107
}

DEFAULT =   'default'
BOLD =      'bold'
DIM =       'dim'
UNDERLINE = 'underlined'
BLINK =     'blink'
REVERSE =   'reverse'
HIDDEN =    'hidden'

BLACK =         'black'
RED =           'red'
GREEN =         'green'
YELLOW =        'yellow'
BLUE =          'blue'
MAGENTA =       'magenta'
CYAN =          'cyan'
LIGHT_GREY =    'lightgrey'
DARK_GREY =     'darkgrey'
LIGHT_RED =     'lightred'
LIGHT_GREEN =   'lightgreen'
LIGHT_YELLOW =  'lightyellow'
LIGHT_BLUE =    'lightblue'
LIGHT_MAGENTA = 'lightmagenta'
LIGHT_CYAN =    'lightcyan'
WHITE =         'white'

class CString:
    
    def __init__(self, s, fg=DEFAULT, bg=DEFAULT, fmt=DEFAULT):
    
        self.data = s
        
        self.foreground = fg
        self.background = bg
        self.formatting = fmt

    
    def __len__(self):
        return len(self.data)


    def __str__(self):
    
        pre = SEQUENCE.format(
            FORMATTING[self.formatting],
            FOREGROUND[self.foreground],
            BACKGROUND[self.background]
        )
        post = '\033[0m'
        return pre + str(self.data) + post

cstr = CString
