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

// Paramètres de la taille du damier
const Size CHECKERBOARD_SIZE(8, 6);  // Coins internes
const float SQUARE_SIZE = 13.0;  // en millimètres
const float distance_between_cameras = 390.0;  // en millimètres

int main() {
    // Critère de terminaison
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
    // Listes pour stocker les points d'objet et d'image pour les deux caméras
    vector<vector<Point3f>> objpoints1, objpoints2;
    vector<vector<Point2f>> imgpoints1, imgpoints2;

    


    // Accéder aux deux caméras
    VideoCapture cap1(0);  // Caméra intégrée
    VideoCapture cap2(1);  // Caméra externe

    if (!cap1.isOpened() || !cap2.isOpened()) {
        cerr << "Erreur de capture vidéo" << endl;
        return -1;
    }

    Mat frame1, frame2, gray1, gray2;

    while (true) 
    {
        // Capture des images des deux caméras
        bool ret1 = cap1.read(frame1);
        bool ret2 = cap2.read(frame2);

        if (!ret1 || !ret2) {
            cerr << "Erreur de capture vidéo" << endl;
            break;
        }

        cvtColor(frame1, gray1, COLOR_BGR2GRAY);
        cvtColor(frame2, gray2, COLOR_BGR2GRAY);

        // Trouver les coins du damier pour chaque caméra
        vector<Point2f> corners1, corners2;
        bool found1 = findChessboardCorners(gray1, CHECKERBOARD_SIZE, corners1, CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_FAST_CHECK + CALIB_CB_NORMALIZE_IMAGE);
        bool found2 = findChessboardCorners(gray2, CHECKERBOARD_SIZE, corners2, CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_FAST_CHECK + CALIB_CB_NORMALIZE_IMAGE);

        if (found1 && found2) {
            // Raffiner les coins pour chaque caméra
            cornerSubPix(gray1, corners1, Size(11, 11), Size(-1, -1), criteria);
            cornerSubPix(gray2, corners2, Size(11, 11), Size(-1, -1), criteria);

            // Afficher les coins détectés
            drawChessboardCorners(frame1, CHECKERBOARD_SIZE, corners1, found1);
            drawChessboardCorners(frame2, CHECKERBOARD_SIZE, corners2, found2);

            // Utiliser le premier coin pour calculer la distance (on peut choisir un autre repère selon le besoin)
            Point2f point1 = corners1[0];  // coin supérieur gauche dans la caméra 1
            Point2f point2 = corners2[0];  // coin supérieur gauche dans la caméra 2

            // Calculer la distance euclidienne en pixels entre les points correspondants
            float pixel_distance = norm(point1 - point2);

            // Calculer la distance entre la plaque et les caméras
            // En supposant que nous connaissons la distance entre les caméras et l'angle, on peut estimer la distance
            float distance_to_board = (distance_between_cameras * SQUARE_SIZE) / pixel_distance;

            // Afficher la distance sur chaque flux vidéo
            putText(frame1, "Distance: " + to_string(distance_to_board) + " mm", Point(50, 50), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 2);
            putText(frame2, "Distance: " + to_string(distance_to_board) + " mm", Point(50, 50), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 2);
        }

        // Afficher les flux en temps réel pour les deux caméras
        imshow("Camera 1", frame1);
        imshow("Camera 2", frame2);

        // Appuyer sur 'q' pour quitter
        if (waitKey(1) == 'q') {
            break;
        }
    }

    // Libérer les captures et fermer les fenêtres
    cap1.release();
    cap2.release();
    destroyAllWindows();

    return 0;
}
