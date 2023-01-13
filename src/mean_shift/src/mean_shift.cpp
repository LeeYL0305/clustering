#define _CRT_SECURE_NO_WARNINGS
#include <ctime> 
#include <fstream>
#include <iostream>
#include <vector>
#include <limits>
#include <float.h>
#include <ros/ros.h>
#include <opencv4/opencv2/opencv.hpp>
#include <opencv4/opencv2/core/core_c.h>

typedef struct Point {
    double x, y;
    int cluster;
    double minDistance;

    Point() :
        x(0.0),
        y(0.0),
        cluster(-1),
        minDistance(std::numeric_limits<double>::max()) {}

    double distance(Point p) {
        return (p.x - x) * (p.x - x) + (p.y - y) * (p.y - y);
    }
};

Point data[100000];
Point centroids[5];

unsigned short pseudo_rand() {
    static unsigned long long seed = 5;
    return (seed = seed * 25214903917ULL + 11ULL) >> 16;
}

void build() {
    int alpha = pseudo_rand();
    int beta = pseudo_rand();

    int loop = 0;
    while (loop < 100000) {
        int tmp = 0;
        for (int i = 0; i < 3; i++) tmp += pseudo_rand() % 400;

        data[loop].x = (double)((tmp / 3 + alpha) % 400) + (pseudo_rand() % 1000000) / 1000000.;
        data[loop].y = (double)((tmp / 3 + beta) % 400) + (pseudo_rand() % 1000000) / 1000000.;
        loop++;
    }
}

void kMeansClustering(int epoch) {
    /* Init cluster */
    for (int i = 0; i < 5; i++)
        centroids[i] = data[pseudo_rand() % 100000];

    while (epoch-- > 0) {
        /* Assign Points to a cluster */
        for (int i = 0; i < 5; i++) {
            for (register int idx = 0; idx < 100000; idx++) {
                double dist = centroids[i].distance(data[idx]);
                if (dist < data[idx].minDistance) {
                    data[idx].minDistance = dist;
                    data[idx].cluster = i;
                }
            }
        }

        /* Computing new centroids */
        int point_cnt[5] = { 0, };
        double sum_x[5] = { 0, };
        double sum_y[5] = { 0, };

        for (register int i = 0; i < 100000; i++) {
            int cluster_id = data[i].cluster;
            point_cnt[cluster_id] += 1;
            sum_x[cluster_id] += data[i].x;
            sum_y[cluster_id] += data[i].y;

            data[i].minDistance = DBL_MAX;
        }

        for (int i = 0; i < 5; i++) {
            centroids[i].x = sum_x[i] / point_cnt[i];
            centroids[i].y = sum_y[i] / point_cnt[i];
        }
    }
}

int main(int argc, char **argv)
{
    ros::init(argc,argv, "mean_shift");
    cv::Mat image = cv::Mat(700, 700, CV_8UC3, cv::Scalar(0, 0, 0));

    build();
    kMeansClustering(100);

    std::ofstream myfile;
    myfile.open("output.csv");
    myfile << "x,y,c" << std::endl;

    for (register int i = 0; i < 100000; i++) {
        myfile << data[i].x << "," << data[i].y << "," << data[i].cluster << std::endl;
        if (data[i].cluster == 0)
        {
            cv::circle(image, cv::Point(data[i].x, data[i].y), 3, CV_RGB(255,0,0), 1);
        }
        if (data[i].cluster == 1)
        {
            cv::circle(image, cv::Point(data[i].x, data[i].y), 3, CV_RGB(255,140,0), 1);
        }
        if (data[i].cluster == 2)
        {
            cv::circle(image, cv::Point(data[i].x, data[i].y), 3, CV_RGB(255,255,0), 1);
        }
         if (data[i].cluster == 3)
        {
            cv::circle(image, cv::Point(data[i].x, data[i].y), 3, CV_RGB(0,255,0), 1);
        }
         if (data[i].cluster == 4)
        {
            cv::circle(image, cv::Point(data[i].x, data[i].y), 3, CV_RGB(0,0,255), 1);
        }
    }
    myfile.close();

    cv::imshow("image", image);
    cv::waitKey(0);

    return 0;
}