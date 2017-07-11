#include <adcs/SunPointing.hpp>
#include <Eigen/Dense>
#include <cstring>
#include <fstream>
#include <iostream>
#include <system.h>
#include <unistd.h>
#include <gtest/gtest.h>
#include "adcs/experimental/adcsUtConfig.h"
#include "adcs/experimental/Include/adcs/dataFileTools.hpp"

using adcs::SunPointing;
using adcs::DipoleVec;
using adcs::MagVec;
using adcs::SunsVec;
using adcs::GyroVec;
using Eigen::Matrix;
using Eigen::Matrix3f;
using Eigen::Vector3f;
using Matrix55f = Matrix<float, 5, 5>;

enum ESPDataIdx
{
    ESP_t = 0,                // 1x1
    ESP_mtmMeas = 1,          // 3x1
    ESP_ssMeas = 4,           // 2x1
    ESP_gyroMeas = 6,         // 3x1
    ESP_mtmFlag = 9,          // 1x1
    ESP_ssFlag = 10,          // 1x1
    ESP_gyroFlag = 11,        // 1x1
    ESP_X_EKF_Prev = 16,      // 5x1
    ESP_p_EKF_Prev = 41,      // 5x5
    ESP_ctrlTorquePrev = 44,  // 3x1
    ESP_EKFconvCountPrev = 45,  // 1x1
    ESP_commDipoleSP = 48,    // 3x1
    ESP_ctrlTorque = 51,      // 3x1
    ESP_X_EKF = 56,           // 5x1
    ESP_P_EKF = 81,           // 5x5
    ESP_innov = 86,           // 5x1
    ESP_innovCov = 111,       // 5x5
    ESP_EKFisInit = 112,      // 1x1
    ESP_EKFisConv = 113,      // 1x1
    ESP_EKFconvCount = 114,   // 1x1
    ESPDataIdx_size
};

// cross-validation of skew function against matlab implementation
TEST(sunpointing, cross_validation_skew)
{
    const uint recordLen = 12;   //in: 3x1 + out: 3x3
    Vector3f in;
    Matrix3f res;
    std::ifstream file(
    ADCS_UT_DATA_FILE_PATH "/sunpointing_crossvalidation_skew.csv");
    if (!file)
    {
        FAIL()<< "Cannot find data  file!" << std::endl;
    }
    std::vector<float> record;

    while (!file.eof())
    //for(int i = 0; i< 2; i++)
    {
        record = dataFileTools::getRecord(file);
        if (record.size() != recordLen)
        {
            if (file.eof())
            {
                break;
            }
            else
            {
                FAIL()<< "Data record has size different than expected (got: " << record.size() << " but expected: " << recordLen << ")" << std::endl;
            }
        }

        in << record[0], record[1], record[2];
        res = skew(in);

        //EXPECT_NEAR(res[0], (Matrix3f << record[3],record[4],record[5],record[6],record[7],record[8],record[9],record[10],record[11]).finished(), 1.0);
        for (int j = 0; j < res.cols(); j++)
        {
            for (int i = 0; i < res.rows(); i++)
            {
                EXPECT_NEAR(res(i, j), record[3 + i + j * res.cols()], 1.0);
#ifdef ADCS_SUNPOINTING_DEBUG
                std::cout << "EXPECT_NEAR(" << res(i, j) << " == " << record[3 + i + j * res.cols()] << ")" << std::endl;
#endif
            }
        }
    }
    file.close();
}

// cross-validation of skew function against matlab implementation
TEST(sunpointing, cross_validation_vecNorm)
{
    const uint recordLen = 4;   //in: 3x1 + out: 1x1
    Vector3f in;
    float res;
    std::ifstream file(
    ADCS_UT_DATA_FILE_PATH "/sunpointing_crossvalidation_vectorNorm.csv");
    if (!file)
    {
        FAIL()<< "Cannot find data  file!" << std::endl;
    }
    std::vector<float> record;

    while (!file.eof())
    //for(int i = 0; i< 2; i++)
    {
        record = dataFileTools::getRecord(file);
        if (record.size() != recordLen)
        {
            if (file.eof())
            {
                break;
            }
            else
            {
                FAIL()<< "Data record has size different than expected (got: " << record.size() << " but expected: " << recordLen << ")" << std::endl;
            }
        }

        in << record[0], record[1], record[2];
        res = in.norm();

        EXPECT_NEAR(res, record[3], 1.0);
//#ifdef ADCS_SUNPOINTING_DEBUG
        std::cout << "EXPECT_NEAR(" << res << " == " << record[3] << ")" << std::endl;
//#endif
    }
    file.close();
}


// cross-validation of matInv function against matlab implementation
TEST(sunpointing, cross_validation_matInv)
{
    const uint recordLen = 50;   //in: 5x5 + out: 5x5
    Matrix55f in;
    Matrix55f res;
    std::ifstream file(
    ADCS_UT_DATA_FILE_PATH "/sunpointing_crossvalidation_matrixInv.csv");
    if (!file)
    {
        FAIL()<< "Cannot find data  file!" << std::endl;
    }
    std::vector<float> record;

    while (!file.eof())
    //for(int i = 0; i< 2; i++)
    {
        record = dataFileTools::getRecord(file);
        if (record.size() != recordLen)
        {
            if (file.eof())
            {
                break;
            }
            else
            {
                FAIL()<< "Data record has size different than expected (got: " << record.size() << " but expected: " << recordLen << ")" << std::endl;
            }
        }

        for (int j = 0; j < res.cols(); j++)
        {
            for (int i = 0; i < res.rows(); i++)
            {
                in(i, j) = record[i + j * res.cols()];
            }
        }
        //res = matInv(in);// accuracy problems
        res = in.inverse();// perfect result

        //EXPECT_NEAR(res[0], (Matrix3f << record[3],record[4],record[5],record[6],record[7],record[8],record[9],record[10],record[11]).finished(), 1.0);
        for (int j = 0; j < res.cols(); j++)
        {
            for (int i = 0; i < res.rows(); i++)
            {
                EXPECT_NEAR(res(i, j), record[25 + i + j * res.cols()], 1.0);
#ifdef ADCS_SUNPOINTING_DEBUG
                std::cout << "EXPECT_NEAR(" << res(i, j) << " == " << record[25 + i + j * res.cols()] << ")" << std::endl;
#endif
            }
        }
    }
    file.close();
}

// cross-validation of sunpointing against matlab implementation
TEST(sunpointing, cross_validation)
{
    std::cout << "SUNPOINTING TEST... NOT REPRESENTATIVE!!!" << std::endl;

    std::ifstream file(
    ADCS_UT_DATA_FILE_PATH "/sunpointing_crossvalidation.csv");
    if (!file)
    {
        FAIL()<< "Cannot find data  file!" << std::endl;
    }
    std::vector<float> record;

    SunPointing::State state; // should be initialised by first measurement
    SunPointing::Parameters params;

    SunPointing sp;
    sp.initialize(state, params);

    // matlab sim is working with different units
    // input: Sim [T] --> OBC [1e-7 T]
    float input_scale = 1.0f;    //1e7f;
    // output: Sim [Am2] --> OBC [1e-4 Am2]
    float output_scale = 1e4f;

    //while (!file.eof())
    for (int i = 0; i < 2; i++)
    {
        record = dataFileTools::getRecord(file); // TODO change to sunpointing data pool
        if (record.size() != ESPDataIdx_size)
        {
            if (file.eof())
            {
                break;
            }
            else
            {
                FAIL()<< "Data record has size different than expected (got: " << record.size() << " but expected: " << ESPDataIdx_size << ")" << std::endl;
            }
        }
//#define ADCS_SUNPOINTING_DEBUG
#ifdef ADCS_SUNPOINTING_DEBUG
        std::cout << "time: " << record[ESP_t] << std::endl;
        std::cout << "in0: " << record[ESP_mtmMeas] << " " << input_scale << " " << record[ESP_mtmMeas] * input_scale << " " << (int)(record[1] * input_scale)
        << std::endl;
        std::cout << "in1: " << record[ESP_ssMeas] << " " << input_scale << " " << record[ESP_ssMeas] * input_scale << " " << (int)(record[2] * input_scale)
        << std::endl;
        std::cout << "in2: " << record[ESP_gyroMeas] << " " << input_scale << " " << record[ESP_gyroMeas] * input_scale << " " << (int)(record[3] * input_scale)
        << std::endl;
        std::cout << "out0: " << record[ESP_commDipoleSP] << " " << output_scale << " " << record[ESP_commDipoleSP] * output_scale << " "
        << (int)(record[ESP_commDipoleSP] * output_scale) << std::endl;
        std::cout << "out1: " << record[ESP_commDipoleSP+1] << " " << output_scale << " " << record[ESP_commDipoleSP+1] * output_scale << " "
        << (int)(record[ESP_commDipoleSP+1] * output_scale) << std::endl;
        std::cout << "out2: " << record[ESP_commDipoleSP+2] << " " << output_scale << " " << record[ESP_commDipoleSP+2] * output_scale << " "
        << (int)(record[ESP_commDipoleSP+2] * output_scale) << std::endl;
#else
        UNUSED1(input_scale);
#endif
        DipoleVec dipole =
        { 0, 0, 0 };
        MagVec mtmMeas =
        { int32_t(record[ESP_mtmMeas + 0]), int32_t(record[ESP_mtmMeas + 1]),
                int32_t(record[ESP_mtmMeas + 2]) };
        bool mtmFlag = (record[ESP_mtmFlag] != 0);
        SunsVec ssMeas =
        { record[ESP_ssMeas + 0], record[ESP_ssMeas + 1] };
        bool ssFlag = (record[ESP_ssFlag] != 0);
        GyroVec gyrMeas =
        { record[ESP_gyroMeas + 0], record[ESP_gyroMeas + 1],
                record[ESP_gyroMeas + 2] };
        bool gyrFlag = (record[ESP_gyroFlag] != 0);
        sp.step(dipole, mtmMeas, mtmFlag, ssMeas, ssFlag, gyrMeas, gyrFlag,
                state);

        EXPECT_NEAR(dipole[0], record[ESP_commDipoleSP + 0] * output_scale,
                1.0);
        EXPECT_NEAR(dipole[1], record[ESP_commDipoleSP + 1] * output_scale,
                1.0);
        EXPECT_NEAR(dipole[2], record[ESP_commDipoleSP + 2] * output_scale,
                1.0);

#ifdef ADCS_SUNPOINTING_DEBUG
        std::cout << "EXPECT_NEAR(" << dipole[0] << " == " << record[ESP_commDipoleSP+0] * output_scale << ")" << std::endl;
        std::cout << "EXPECT_NEAR(" << dipole[1] << " == " << record[ESP_commDipoleSP+1] * output_scale << ")" << std::endl;
        std::cout << "EXPECT_NEAR(" << dipole[2] << " == " << record[ESP_commDipoleSP+2] * output_scale << ")" << std::endl;
#endif
    }
    file.close();
}