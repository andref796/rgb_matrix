import sys
from rgb_matrix.rgb_matrix import RGB_Matrix
import time

if __name__ == "__main__":
    rr = RGB_Matrix(0X74)

    while True:
        rr.show_text("  Hello  ",delay=200,color=(0,51,128))
        time.sleep(0.2)