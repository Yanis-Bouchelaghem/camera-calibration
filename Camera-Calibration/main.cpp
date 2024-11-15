#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include<stdio.h>
#include "Camera.h"

using namespace cv;
using namespace std;

// Param�tres de la taille du damier
const Size CHECKERBOARD_SIZE(8, 6);  // Coins internes
const float SQUARE_SIZE = 13.0;  // en millim�tres
const float distance_between_cameras = 390.0;  // en millim�tres

int main() {
    // Crit�re de terminaison
    TermCriteria criteria(TermCriteria::EPS + TermCriteria::MAX_ITER, 30, 0.001);
    Camera myCamera(CHECKERBOARD_SIZE,
                    SQUARE_SIZE,
                    distance_between_cameras,
                    1,
                    criteria);
    while (true)
    {
        auto corners = myCamera.Tick();
        myCamera.Draw(corners);
        // Appuyer sur 'q' pour quitter
        if (waitKey(1) == 'q') {
            break;
        }
    }
    // Listes pour stocker les points d'objet et d'image pour les deux cam�ras
    vector<vector<Point3f>> objpoints1, objpoints2;
    vector<vector<Point2f>> imgpoints1, imgpoints2;

    


    // Acc�der aux deux cam�ras
    VideoCapture cap1(0);  // Cam�ra int�gr�e
    VideoCapture cap2(1);  // Cam�ra externe

    if (!cap1.isOpened() || !cap2.isOpened()) {
        cerr << "Erreur de capture vid�o" << endl;
        return -1;
    }

    Mat frame1, frame2, gray1, gray2;

    while (true) 
    {
        // Capture des images des deux cam�ras
        bool ret1 = cap1.read(frame1);
        bool ret2 = cap2.read(frame2);

        if (!ret1 || !ret2) {
            cerr << "Erreur de capture vid�o" << endl;
            break;
        }

        cvtColor(frame1, gray1, COLOR_BGR2GRAY);
        cvtColor(frame2, gray2, COLOR_BGR2GRAY);

        // Trouver les coins du damier pour chaque cam�ra
        vector<Point2f> corners1, corners2;
        bool found1 = findChessboardCorners(gray1, CHECKERBOARD_SIZE, corners1, CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_FAST_CHECK + CALIB_CB_NORMALIZE_IMAGE);
        bool found2 = findChessboardCorners(gray2, CHECKERBOARD_SIZE, corners2, CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_FAST_CHECK + CALIB_CB_NORMALIZE_IMAGE);

        if (found1 && found2) {
            // Raffiner les coins pour chaque cam�ra
            cornerSubPix(gray1, corners1, Size(11, 11), Size(-1, -1), criteria);
            cornerSubPix(gray2, corners2, Size(11, 11), Size(-1, -1), criteria);

            // Afficher les coins d�tect�s
            drawChessboardCorners(frame1, CHECKERBOARD_SIZE, corners1, found1);
            drawChessboardCorners(frame2, CHECKERBOARD_SIZE, corners2, found2);

            // Utiliser le premier coin pour calculer la distance (on peut choisir un autre rep�re selon le besoin)
            Point2f point1 = corners1[0];  // coin sup�rieur gauche dans la cam�ra 1
            Point2f point2 = corners2[0];  // coin sup�rieur gauche dans la cam�ra 2

            // Calculer la distance euclidienne en pixels entre les points correspondants
            float pixel_distance = norm(point1 - point2);

            // Calculer la distance entre la plaque et les cam�ras
            // En supposant que nous connaissons la distance entre les cam�ras et l'angle, on peut estimer la distance
            float distance_to_board = (distance_between_cameras * SQUARE_SIZE) / pixel_distance;

            // Afficher la distance sur chaque flux vid�o
            putText(frame1, "Distance: " + to_string(distance_to_board) + " mm", Point(50, 50), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 2);
            putText(frame2, "Distance: " + to_string(distance_to_board) + " mm", Point(50, 50), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 2);
        }

        // Afficher les flux en temps r�el pour les deux cam�ras
        imshow("Camera 1", frame1);
        imshow("Camera 2", frame2);

        // Appuyer sur 'q' pour quitter
        if (waitKey(1) == 'q') {
            break;
        }
    }

    // Lib�rer les captures et fermer les fen�tres
    cap1.release();
    cap2.release();
    destroyAllWindows();

    return 0;
}
