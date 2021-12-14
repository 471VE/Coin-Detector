IMPORTANT: this program needs C++17 to compile and run. This is due to having <filesystem> included in "src/helpers.cpp". For older versions of C++, you might need to change "#include <filesystem>" to "#include <experimantal/filesystem>" and "std::filesystem" to "std::experimental::filesystem" in "src/helpers.cpp".
OpenCV library must also be installed and added to environment variables.

This program detects coins using CHT (circle Hough Transform).

Build the program using CMake.
To run the program, specify the path to the executable and add path to the image or to the folder with images. By default, it is assumed that .txt files with true circle parameters are present in the "parameters" subdirectory of the same directory as the image to be processed. You need to add flag "--skip-f1" to prevent the error from being raised.
By default, IoU metric is used for evaluating whether the coin is detected correctly or not with the threshold 0.6. If you wish to change this threshold, add flag "IoU x", where x is your chose threshold value between 0 and 1. If you wish to check all parameters of circles independently, add flag "independent".

Examples for Windows system (copy and paste to the command line without quotes):
- "build\bin\Release\detector images";
- "build\bin\Release\detector images --skip-f1";
- "build\bin\Release\detector images independent";
- "build\bin\Release\detector images IoU 0.6";
- "build\bin\Release\detector images\img001.jpg";
- "build\bin\Release\detector images\img001.jpg --skip-f1";
- "build\bin\Release\detector images\img001.jpg independent";
- "build\bin\Release\detector images\img001.jpg IoU 0.6";

To go the next image when running the program, hit any key.

F1-score is calculated by default. For that to work, there must be .txt file with circle parameters in the "parameters" subdirectory of the same directory as the image being processed and with the SAME name (except for extension). Otherwise, the error is raised. This .txt file should have the following structure:

x_coordinate_of_the_center1 y_coordinate_of_the_center1 radius1
x_coordinate_of_the_center2 y_coordinate_of_the_center2 radius2
...
x_coordinate_of_the_centerN y_coordinate_of_the_centerN radiusN

The values must be separated by single space and parameters of different circles should be specified on different lines. You can open "images\parameters\img001.txt" to see the example.

You can do that automatically by drawing diameter of the circle with the mouse using "utilities\writetruecircles.py" Python script. To use this script you need to have NumPy and OpenCV installed. To install these libraries run "pip install numpy" and "pip install opencv-python" commands in the terminal.
To run the script, place the images in "test_images" directory and run "python utilities\writetruecircles.py test_images". When you are done drawing circles on the image, hit Space, Escape, or Enter key to go to the next image. If you do not draw any circles on the image, the file with circle parameters will not be created.

If the directory is specified when running the program, overall F1-score over all processed images will be the last line in the terminal.
All images with detected circles and logs are saved in "path_to_image\detected\".

To test the program on your dataset, copy your images to "test_images" directory with corresponding .txt files in "test_images\parameters" and run: "build\bin\Release\detector test_images" (for Windows machines).