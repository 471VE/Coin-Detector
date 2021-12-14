import cv2
import numpy as np
from glob import glob
from os.path import isdir
from os import makedirs
from sys import argv

class IncorrectNumberOfArguments(Exception):
    pass

class NoDirectory(Exception):
    pass

def center_radius(ix, x, iy, y):
    x_center = round((ix + x) / 2)
    y_center = round((iy + y) / 2)
    radius = round(np.sqrt((x - ix)**2 + (y - iy)**2) / 2)
    return (x_center, y_center), radius

def txt_filename(image_filename):
    path = image_filename.split("\\")
    path[-1] =  f"parameters\\{path[-1][:-4]}.txt"
    return "\\".join(path)

if __name__ == "__main__":
    
    if len(argv) == 1:
        raise IncorrectNumberOfArguments('The path to images should be specified.')
    elif len(argv) > 2:
        raise IncorrectNumberOfArguments('Only one directory can be processed at a time.')
    
    directory = argv[1]
    if not isdir(directory):
        raise NoDirectory('No such directory exists.')
    
    is_drawing = False
    ix = -1
    iy = -1
    image_names = glob(f'{directory}\\*.jpg') + glob(f'{directory}\\*.png')

    for image_name in image_names:
        image = cv2.imread(image_name)
        scale = 1
        if image.shape[1] > 1500 or image.shape[0] > 800:
            scale_x = 1500 / image.shape[1]
            scale_y = 800 / image.shape[0]
            scale = min(scale_x, scale_y)
            width = int(image.shape[1] * scale)
            height = int(image.shape[0] * scale)
            image = cv2.resize(image, (width, height))
        cache = image.copy()
        circle_parameters = ""

        def draw_circle(event, x, y, flags, param):
            global is_drawing, ix, iy
            global image, cache, circle_parameters
            
            if event == cv2.EVENT_LBUTTONDOWN:
                is_drawing = True
                ix = x
                iy = y
                
            elif event == cv2.EVENT_LBUTTONUP:
                is_drawing = False
                center, radius = center_radius(ix, x, iy, y)
                cv2.circle(img=image, center=center, radius=radius, color=(0, 0, 255), thickness=2)
                cv2.circle(img=image, center=center, radius=2, color=(0, 255, 0), thickness=-1)
                circle_parameters += f"{round(center[0]/scale)} {round(center[1]/scale)} {round(radius/scale)}\n"
                cache = image.copy()
                ix = -1
                iy = -1
                
            elif event == cv2.EVENT_MOUSEMOVE:
                if is_drawing:
                    image = cache.copy()
                    center, radius = center_radius(ix, x, iy, y)
                    cv2.circle(img=image, center=center, radius=radius, color=(0, 0, 255), thickness=2)
                    cv2.circle(img=image, center=center, radius=2, color=(0, 255, 0), thickness=-1)
                    
        cv2.namedWindow(image_name)
        cv2.setMouseCallback(image_name, draw_circle)

        while True:
            cv2.imshow(image_name, image)
            if cv2.waitKey(5) & 0xFF in (13, 32, 27):
                break
        cv2.destroyAllWindows()
        
        if circle_parameters:
            if not isdir(f"{directory}\\parameters"):
                makedirs(f"{directory}\\parameters")
            with open(txt_filename(image_name), 'w') as txt_file:
                txt_file.write(circle_parameters[:-1])