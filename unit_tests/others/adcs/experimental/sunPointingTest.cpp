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

static const float COMP_ACCUR = 0.0001;

using adcs::SunPointing;
using adcs::DipoleVec;
using adcs::MagVec;
using adcs::SunsVec;
using adcs::GyroVec;
using Eigen::Matrix;
using Eigen::Vector3f;
using Matrix3f = Matrix<float, 3, 3, Eigen::RowMajor>;
using Matrix5f = Matrix<float, 5, 5, Eigen::RowMajor>;

enum ESPDataIdx
{
    ESP_t = 0,                // 1x1
    ESP_mtmMeas = 1,          // 3x1
    ESP_ssMeas = 4,           // 2x1
    ESP_gyroMeas = 6,         // 3x1
    ESP_mtmFlag = 9,          // 1x1
    ESP_ssFlag = 10,          // 1x1
    ESP_gyroFlag = 11,        // 1x1
    ESP_X_EKF_Prev = 12,      // 5x1
    ESP_p_EKF_Prev = 17,      // 5x5
    ESP_ctrlTorquePrev = 42,  // 3x1
    ESP_EKFconvCountPrev = 45,  // 1x1
    ESP_commDipoleSP = 46,    // 3x1
    ESP_ctrlTorque = 49,      // 3x1
    ESP_X_EKF = 52,           // 5x1
    ESP_P_EKF = 57,           // 5x5
    ESP_innov = 82,           // 5x1
    ESP_innovCov = 87,       // 5x5
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

        //EXPECT_NEAR(res[0], (Matrix3f << record[3],record[4],record[5],record[6],record[7],record[8],record[9],record[10],record[11]).finished(), COMP_ACCUR);
        for (int j = 0; j < res.cols(); j++)
        {
            for (int i = 0; i < res.rows(); i++)
            {
                EXPECT_NEAR(res(i, j), record[3 + i + j * res.cols()],
                        COMP_ACCUR);
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

        EXPECT_NEAR(res, record[3], COMP_ACCUR);
#ifdef ADCS_SUNPOINTING_DEBUG
        std::cout << "EXPECT_NEAR(" << res << " == " << record[3] << ")" << std::endl;
#endif
    }
    file.close();
}

// cross-validation of matInv function against matlab implementation
TEST(sunpointing, cross_validation_matInv)
{
    const float COMP_ACCUR = 0.01;   //XXX decreased accuracy

    const uint recordLen = 75;   //in: 5x5 + out: 5x5 + 5x5 matlab algo output
    Matrix5f in;
    Matrix5f res;
    Matrix5f res2;
    Matrix5f resMatlab;

    std::ifstream file(
    ADCS_UT_DATA_FILE_PATH "/sunpointing_crossvalidation_matrixInv.csv");
    if (!file)
    {
        FAIL()<< "Cannot find data  file!" << std::endl;
    }
    std::vector<float> record;

    while (!file.eof())
    //for(int i = 0; i< 100; i++)
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

        for (int j = 0; j < in.cols(); j++)
        {
            for (int i = 0; i < in.rows(); i++)
            {
                in(i, j) = record[i + j * in.cols()];
            }
        }
        res2 = matInv(in);   // accuracy problems with original method
        res = in.inverse();   //

        //         for (int j = 0; j < res.cols(); j++)
        //         {
        //         for (int i = 0; i < res.rows(); i++)
        //         {
        //         EXPECT_NEAR(res(i, j), res2(i,j),
        //         COMP_ACCUR);
        //         #ifdef ADCS_SUNPOINTING_DEBUG
        //         std::cout << "EXPECT_NEAR(" << res(i, j) << " == " << res(i, j) << ")" << std::endl;
        //         #endif
        //         }
        //         }

        //EXPECT_NEAR(res[0], (Matrix3f << record[3],record[4],record[5],record[6],record[7],record[8],record[9],record[10],record[11]).finished(), COMP_ACCUR);
        for (int j = 0; j < res.cols(); j++)
        {
            for (int i = 0; i < res.rows(); i++)
            {
                EXPECT_NEAR(res(i, j), record[25 + i + j * res.cols()],
                        COMP_ACCUR);
#ifdef ADCS_SUNPOINTING_DEBUG
                std::cout << "EXPECT_NEAR(" << res(i, j) << " == " << record[25 + i + j * res.cols()] << ")" << std::endl;
#endif
            }
        }
        for (int j = 0; j < res.cols(); j++)
        {
            for (int i = 0; i < res.rows(); i++)
            {
                EXPECT_NEAR(res(i, j), record[50 + i + j * res.cols()],
                        COMP_ACCUR);
#ifdef ADCS_SUNPOINTING_DEBUG
                std::cout << "EXPECT_NEAR(" << res(i, j) << " == " << record[50 + i + j * res.cols()] << ")" << std::endl;
#endif
            }
        }

    }
    file.close();
}

// cross-validation of spinControler function against matlab implementation
TEST(sunpointing, cross_validation_spinCtrl)
{
    const uint recordLen = 12; // 4x(3x1)
    Vector3f inAngRateEst;
    Vector3f inBodyEst;
    Vector3f inMtmMeas;
    Vector3f outDipole;
    SunPointing::State state; //just parameterisation
    std::ifstream file(
    ADCS_UT_DATA_FILE_PATH "/sunpointing_crossvalidation_SpinController.csv");
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

        for (int j = 0; j < inAngRateEst.cols(); j++)
        {
            for (int i = 0; i < inAngRateEst.rows(); i++) //TODO do sth about indexing
            {
                inAngRateEst(i, j) = record[0 + i + j * inAngRateEst.cols()];
                inBodyEst(i, j) = record[3 + i + j * inAngRateEst.cols()];
                inMtmMeas(i, j) = record[6 + i + j * inAngRateEst.cols()];
            }
        }

        SpinController(outDipole, inAngRateEst, inBodyEst, inMtmMeas, state);

        //EXPECT_NEAR(res[0], (Matrix3f << record[3],record[4],record[5],record[6],record[7],record[8],record[9],record[10],record[11]).finished(), COMP_ACCUR);
        for (int j = 0; j < outDipole.cols(); j++)
        {
            for (int i = 0; i < outDipole.rows(); i++)
            {
                EXPECT_NEAR(outDipole(i, j),
                        record[9 + i + j * outDipole.cols()], COMP_ACCUR);
#ifdef ADCS_SUNPOINTING_DEBUG
                std::cout << "EXPECT_NEAR(" << outDipole(i, j) << " == " << record[9 + i + j * outDipole.cols()] << ")" << std::endl;
#endif
            }
        }
    }
    file.close();
}

// cross-validation of EKFInitialisation function against matlab implementation
TEST(sunpointing, cross_validation_ekfInit)
{
    uint record_cntr = 0;
    const uint recordLen = 5 + 25 + 5 + 25 + 2 + 3;
    //inputs
    Vector2f ssMeas;
    Vector3f gyrMeas;
    //outputs
    Vector5f xEkf;
    Matrix5f pEkf;
    Vector5f innov;
    Matrix5f innovCov;

    SunPointing::State state; //just parameterisation

    std::ifstream file(
    ADCS_UT_DATA_FILE_PATH "/sunpointing_crossvalidation_init.csv");
    if (!file)
    {
        FAIL()<< "Cannot find data  file!" << std::endl;
    }
    std::vector<float> record;

    while (!file.eof())
    //for(int i = 0; i< 2; i++)
    {
        record_cntr = 0;
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

        ssMeas[0] = record[record_cntr++]; //TODO put this to function!!!
        ssMeas[1] = record[record_cntr++];
        gyrMeas[0] = record[record_cntr++];
        gyrMeas[1] = record[record_cntr++];
        gyrMeas[2] = record[record_cntr++];

        EKFinitialization(xEkf, pEkf, innov, innovCov, ssMeas, gyrMeas, state);

        //EXPECT_NEAR(res[0], (Matrix3f << record[3],record[4],record[5],record[6],record[7],record[8],record[9],record[10],record[11]).finished(), COMP_ACCUR);
        for (int j = 0; j < xEkf.cols(); j++) //TODO put this test to function!!!
        {
            for (int i = 0; i < xEkf.rows(); i++)
            {
                EXPECT_NEAR(xEkf(i, j), record[record_cntr], COMP_ACCUR);
#ifdef ADCS_SUNPOINTING_DEBUG
                std::cout << "EXPECT_NEAR(" << xEkf(i, j) << " == "
                << record[record_cntr] << ")" << std::endl;
#endif
                record_cntr++;
            }
        }

        for (int j = 0; j < pEkf.cols(); j++)
        {
            for (int i = 0; i < pEkf.rows(); i++)
            {
                EXPECT_NEAR(pEkf(i, j), record[record_cntr], COMP_ACCUR);
#ifdef ADCS_SUNPOINTING_DEBUG
                std::cout << "EXPECT_NEAR(" << pEkf(i, j) << " == " << record[record_cntr] << ")" << std::endl;
#endif
                record_cntr++;
            }
        }

        for (int j = 0; j < innov.cols(); j++)
        {
            for (int i = 0; i < innov.rows(); i++)
            {
                EXPECT_NEAR(innov(i, j), record[record_cntr], COMP_ACCUR);
#ifdef ADCS_SUNPOINTING_DEBUG
                std::cout << "EXPECT_NEAR(" << innov(i, j) << " == "
                << record[record_cntr] << ")" << std::endl;
#endif
                record_cntr++;
            }
        }

        for (int j = 0; j < innovCov.cols(); j++)
        {
            for (int i = 0; i < innovCov.rows(); i++)
            {
                EXPECT_NEAR(innovCov(i, j), record[record_cntr], COMP_ACCUR);
#ifdef ADCS_SUNPOINTING_DEBUG
                std::cout << "EXPECT_NEAR(" << innovCov(i, j) << " == " << record[record_cntr] << ")" << std::endl;
#endif
                record_cntr++;
            }
        }
    }
    file.close();
}

// cross-validation of all components of sunVandRate function against matlab implementation
TEST(sunpointing, cross_validation_sunVandRateExtendedInDepth)
{
    uint record_cntr = 0;
    //inputs
    Vector5f xEkf;
    float sslat;
    float sslong;
    //outputs
    float sinlat;
    float sinlong;
    float coslat;
    float coslong;
    Matrix3f rotSSt_exp;
    Vector3f SunVector_SS_exp;
    Vector3f s2s_bodyEst_exp;
    Vector3f angrateEst_exp;

    SunPointing::State state; //just parameterisation

    const uint recordLen = 5 + 1 + 1 + 1 + 1 + 1 + 1 + 9 + 3 + 3 + 3; //TODO dep on size of above

    std::ifstream file(
    ADCS_UT_DATA_FILE_PATH "/sunpointing_crossvalidation_sin.csv"); //TODO change file name
    if (!file)
    {
        FAIL()<< "Cannot find data  file!" << std::endl;
    }
    std::vector<float> record;

    while (!file.eof())
    //for (int i = 0; i < 10; i++)//TODO remove
    {
        record_cntr = 0;
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

        for (int j = 0; j < xEkf.cols(); j++) //TODO put this test to function!!!
        {
            for (int i = 0; i < xEkf.rows(); i++)
            {
                xEkf(i, j) = record[record_cntr++];
            }
        }

        sslat = record[record_cntr++];
        sslong = record[record_cntr++];

        float SS_lat = xEkf(0, 0);
        float SS_long = xEkf(1, 0);

        sinlat = sinf(sslat);
        sinlong = sinf(sslong);
        coslat = cosf(sslat);
        coslong = cosf(sslong);

        Vector3f SunVector_SS;
        SunVector_SS << sinf(SS_lat) * cosf(SS_long), sinf(SS_lat)
                * sinf(SS_long), cosf(SS_lat);

        Matrix3f rotSSt;
        rotSSt = Matrix3f(state.params.rotSS.data()).transpose();

        Vector3f s2s_bodyEst;
        s2s_bodyEst = Matrix3f(state.params.rotSS.data()).transpose()
                * SunVector_SS;

        Vector3f angrateEst;
        angrateEst = xEkf.block(2, 0, 3, 1);

        EXPECT_NEAR(sslat, SS_lat, COMP_ACCUR);
        EXPECT_NEAR(sslong, SS_long, COMP_ACCUR);

        EXPECT_NEAR(sinlat, record[record_cntr], COMP_ACCUR);
#ifdef ADCS_SUNPOINTING_DEBUG
        std::cout << "EXPECT_NEAR(" << sinlat << " == "
        << record[record_cntr] << ")" << std::endl;
#endif
        record_cntr++;

        EXPECT_NEAR(coslat, record[record_cntr], COMP_ACCUR);
#ifdef ADCS_SUNPOINTING_DEBUG
        std::cout << "EXPECT_NEAR(" << coslat << " == "
        << record[record_cntr] << ")" << std::endl;
#endif
        record_cntr++;

        EXPECT_NEAR(coslong, record[record_cntr], COMP_ACCUR);
#ifdef ADCS_SUNPOINTING_DEBUG
        std::cout << "EXPECT_NEAR(" << coslong << " == "
        << record[record_cntr] << ")" << std::endl;
#endif
        record_cntr++;

        EXPECT_NEAR(sinlong, record[record_cntr], COMP_ACCUR);
#ifdef ADCS_SUNPOINTING_DEBUG
        std::cout << "EXPECT_NEAR(" << sinlong << " == "
        << record[record_cntr] << ")" << std::endl;
#endif
        record_cntr++;

        for (int j = 0; j < rotSSt.cols(); j++)
        {
            for (int i = 0; i < rotSSt.rows(); i++)
            {
                EXPECT_NEAR(rotSSt(i, j), record[record_cntr], COMP_ACCUR); //XXX only one with changed indexing
#ifdef ADCS_SUNPOINTING_DEBUG
                std::cout << "EXPECT_NEAR(" << rotSSt(j, i) << " == "<< record[record_cntr] << ")" << std::endl;
#endif
                record_cntr++;
            }
        }

        for (int j = 0; j < SunVector_SS.cols(); j++)
        {
            for (int i = 0; i < SunVector_SS.rows(); i++)
            {
                EXPECT_NEAR(SunVector_SS(i, j), record[record_cntr],
                        COMP_ACCUR);
#ifdef ADCS_SUNPOINTING_DEBUG
                std::cout << "EXPECT_NEAR(" << SunVector_SS(i, j) << " == "<< record[record_cntr] << ")" << std::endl;
#endif
                record_cntr++;
            }
        }

        for (int j = 0; j < s2s_bodyEst.cols(); j++)
        {
            for (int i = 0; i < s2s_bodyEst.rows(); i++)
            {
                EXPECT_NEAR(s2s_bodyEst(i, j), record[record_cntr], COMP_ACCUR);
#ifdef ADCS_SUNPOINTING_DEBUG
                std::cout << "EXPECT_NEAR(" << s2s_bodyEst(i, j) << " == "<< record[record_cntr] << ")" << std::endl;
#endif
                record_cntr++;
            }
        }

        for (int j = 0; j < angrateEst.cols(); j++)
        {
            for (int i = 0; i < angrateEst.rows(); i++)
            {
                EXPECT_NEAR(angrateEst(i, j), record[record_cntr], COMP_ACCUR);
#ifdef ADCS_SUNPOINTING_DEBUG
                std::cout << "EXPECT_NEAR(" << angrateEst(i, j) << " == " << record[record_cntr] << ")" << std::endl;
#endif
                record_cntr++;
            }
        }
    }

    file.close();
}

// cross-validation of ExtractSunVandRate function against matlab implementation
TEST(sunpointing, cross_validation_sunVandRate)
{
    SunPointing::State state; //just parameterisation

    uint record_cntr = 0;
    //inputs
    Vector5f xEkf;
    //outputs
    Vector3f s2s_bodyEst;
    Vector3f angrateEst;

    const uint recordLen = 5 + 3 + 3; //TODO dep on size of above

    std::ifstream file(
            ADCS_UT_DATA_FILE_PATH "/sunpointing_crossvalidation_ExtractSunVandRate.csv");
    if (!file)
    {
        FAIL()<< "Cannot find data  file!" << std::endl;
    }
    std::vector<float> record;

    while (!file.eof())
    //for(int i = 0; i< 2; i++)
    {
        record_cntr = 0;
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

        for (int j = 0; j < xEkf.cols(); j++) //TODO put this test to function!!!
        {
            for (int i = 0; i < xEkf.rows(); i++)
            {
                xEkf(i, j) = record[record_cntr++];
            }
        }

        ExtractSunVandRate(s2s_bodyEst, angrateEst, xEkf, state);

        for (int j = 0; j < s2s_bodyEst.cols(); j++)
        {
            for (int i = 0; i < s2s_bodyEst.rows(); i++)
            {
                EXPECT_NEAR(s2s_bodyEst(i, j), record[record_cntr], COMP_ACCUR);
#ifdef ADCS_SUNPOINTING_DEBUG
                std::cout << "EXPECT_NEAR(" << s2s_bodyEst(i, j) << " == "<< record[record_cntr] << ")" << std::endl;
#endif
                record_cntr++;
            }
        }

        for (int j = 0; j < angrateEst.cols(); j++)
        {
            for (int i = 0; i < angrateEst.rows(); i++)
            {
                EXPECT_NEAR(angrateEst(i, j), record[record_cntr], COMP_ACCUR);
#ifdef ADCS_SUNPOINTING_DEBUG
                std::cout << "EXPECT_NEAR(" << angrateEst(i, j) << " == " << record[record_cntr] << ")" << std::endl;
#endif
                record_cntr++;
            }
        }
    }
    file.close();
}

// cross-validation of PropagateState function against matlab implementation
TEST(sunpointing, cross_validation_propState)
{
    const float COMP_ACCUR = 0.001; //XXX accuracy decreased

    SunPointing::State state; //just parameterisation TODO const?? or const ref

    uint record_cntr = 0;
    //inputs
    Vector5f xEkfPrev;
    Vector3f ctrlTorquePrev;
    //outputs
    Vector5f k1;

    const uint recordLen = 5 + 3 + 5; //TODO dep on size of above

    std::ifstream file(
    ADCS_UT_DATA_FILE_PATH "/sunpointing_crossvalidation_PropagateState.csv");
    if (!file)
    {
        FAIL()<< "Cannot find data  file!" << std::endl;
    }
    std::vector<float> record;

    while (!file.eof())
    //for (int i = 0; i < 10; i++)
    {
        record_cntr = 0;
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

        for (int j = 0; j < xEkfPrev.cols(); j++) //TODO put this test to function!!!
        {
            for (int i = 0; i < xEkfPrev.rows(); i++)
            {
                xEkfPrev(i, j) = record[record_cntr++];
            }
        }

        for (int j = 0; j < ctrlTorquePrev.cols(); j++) //TODO put this test to function!!!
        {
            for (int i = 0; i < ctrlTorquePrev.rows(); i++)
            {
                ctrlTorquePrev(i, j) = record[record_cntr++];
            }
        }

        k1 = PropagateState(xEkfPrev, ctrlTorquePrev, state);

        for (int j = 0; j < k1.cols(); j++)
        {
            for (int i = 0; i < k1.rows(); i++)
            {
                EXPECT_NEAR(k1(i, j), record[record_cntr], COMP_ACCUR);
#ifdef ADCS_SUNPOINTING_DEBUG
                std::cout << "EXPECT_NEAR(" << k1(i, j) << " == "
                << record[record_cntr] << ")" << std::endl;
#endif
                record_cntr++;
            }
        }
    }
    file.close();
}

// cross-validation of ExtendedKalmanFilter function against matlab implementation
TEST(sunpointing, cross_validation_ekf)
{
    //return;//XXX

    const float COMP_ACCUR = 1.01;

    SunPointing::State state;

    uint record_cntr = 0;
    //inputs
    Vector2f inSsMeas;
    Vector3f inGyrMeas;
    bool ssFlag;
    bool gyrFlag;
    //outputs
    Vector5f xEkf;
    Matrix5f pEkf;
    Vector5f innov;
    Matrix5f innovCov;

    const uint recordLen = 2 + 3 + 1 + 1 + 5 + 25 + 3 + 5 + 25 + 5 + 25; //TODO dep on size of above

    std::ifstream file(
    ADCS_UT_DATA_FILE_PATH "/sunpointing_crossvalidation_ekf.csv");
    if (!file)
    {
        FAIL()<< "Cannot find data  file!" << std::endl;
    }
    std::vector<float> record;

    while (!file.eof())
    //for (int i = 0; i < 10; i++)
    {
        record_cntr = 0;
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

        for (int j = 0; j < inSsMeas.cols(); j++) //TODO put this test to function!!!
        {
            for (int i = 0; i < inSsMeas.rows(); i++)
            {
                inSsMeas(i, j) = record[record_cntr++];
            }
        }

        for (int j = 0; j < inGyrMeas.cols(); j++) //TODO put this test to function!!!
        {
            for (int i = 0; i < inGyrMeas.rows(); i++)
            {
                inGyrMeas(i, j) = record[record_cntr++];
            }
        }

        ssFlag = record[record_cntr++];
        gyrFlag = record[record_cntr++];

        for (int j = 0; j < state.xEkfPrev.cols(); j++) //TODO put this test to function!!!
        {
            for (int i = 0; i < state.xEkfPrev.rows(); i++)
            {
                state.xEkfPrev(i, j) = record[record_cntr++];
            }
        }

        for (int j = 0; j < state.pEkfPrev.cols(); j++) //TODO put this test to function!!!
        {
            for (int i = 0; i < state.pEkfPrev.rows(); i++)
            {
                state.pEkfPrev(i, j) = record[record_cntr++];
            }
        }

        for (int j = 0; j < state.ctrlTorquePrev.cols(); j++) //TODO put this test to function!!!
        {
            for (int i = 0; i < state.ctrlTorquePrev.rows(); i++)
            {
                state.ctrlTorquePrev(i, j) = record[record_cntr++];
            }
        }

        ExtendedKalmanFilter(xEkf, pEkf, innov, innovCov, inSsMeas, ssFlag,
                inGyrMeas, gyrFlag, state);

        for (int j = 0; j < xEkf.cols(); j++)
        {
            for (int i = 0; i < xEkf.rows(); i++)
            {
                EXPECT_NEAR(xEkf(i, j), record[record_cntr], COMP_ACCUR);
#ifdef ADCS_SUNPOINTING_DEBUG
                std::cout << "EXPECT_NEAR(" << xEkf(i, j) << " == "
                << record[record_cntr] << ")" << std::endl;
#endif
                record_cntr++;
            }
        }

        for (int j = 0; j < pEkf.cols(); j++)
        {
            for (int i = 0; i < pEkf.rows(); i++)
            {
                EXPECT_NEAR(pEkf(i, j), record[record_cntr], COMP_ACCUR);
#ifdef ADCS_SUNPOINTING_DEBUG
                std::cout << "EXPECT_NEAR(" << pEkf(i, j) << " == "
                << record[record_cntr] << ")" << std::endl;
#endif
                record_cntr++;
            }
        }

        for (int j = 0; j < innov.cols(); j++)
        {
            for (int i = 0; i < innov.rows(); i++)
            {
                EXPECT_NEAR(innov(i, j), record[record_cntr], COMP_ACCUR);
#ifdef ADCS_SUNPOINTING_DEBUG
                std::cout << "EXPECT_NEAR(" << innov(i, j) << " == "
                << record[record_cntr] << ")" << std::endl;
#endif
                record_cntr++;
            }
        }

        for (int j = 0; j < innovCov.cols(); j++)
        {
            for (int i = 0; i < innovCov.rows(); i++)
            {
                EXPECT_NEAR(innovCov(i, j), record[record_cntr], COMP_ACCUR);
#ifdef ADCS_SUNPOINTING_DEBUG
                std::cout << "EXPECT_NEAR(" << innovCov(i, j) << " == "
                << record[record_cntr] << ")" << std::endl;
#endif
                record_cntr++;
            }
        }
    }
    file.close();
}
//*/

// cross-validation of ExtendedKalmanFilter function against matlab implementation
TEST(sunpointing, cross_validation_ekf_indepth)
{
    //return;//XXX

    const float COMP_ACCUR = 1e-4;

    SunPointing::State state;

    uint record_cntr = 0;

    //Vector5f X_EKF_Prev;//state
    //Matrix5f P_EKF_Prev;//state
    Vector2f ssMeas;
    Vector3f gyrMeas;
    bool ssFlag;
    bool gyrFlag;
    //Vector3f ctrlTorquePrev;//state
    Vector5f xPrio_exp;
    Vector3f sv_SS_exp;
    RowVector3f dLong_dSS_exp;
    Matrix3f angrate_ss_exp;
    Matrix5f jacobianF_exp;
    Matrix5f P_prio_exp;
    Matrix5f innovCov_exp;
    Matrix5f innovCovInv_exp;
    Matrix5f K_exp;
    Vector5f innov_exp;
    Vector5f xEkf_exp;
    Matrix5f jf_m_exp;
    Matrix5f pEkf_exp;

    Vector5f xEkf;
    Matrix5f pEkf;
    Vector5f innov;
    Matrix5f innovCov;
    Matrix5f innovCovInv;

    const uint recordLen = 5 + 25 + 2 + 3 + 1 + 1 + 3 + 5 + 3 + 3 + 9 + 5 * 25
            + 5 + 5 + 25 + 25; //TODO dep on size of above

    std::ifstream file(
    ADCS_UT_DATA_FILE_PATH "/sunpointing_crossvalidation_ekf_indepth.csv");
    if (!file)
    {
        FAIL()<< "Cannot find data  file!" << std::endl;
    }
    std::vector<float> record;

    int LoopCntr = -1;

    while (!file.eof())
    //for (int i = 0; i < 10; i++)
    {
        LoopCntr++;
        std::cout << "LoopCntr [" << LoopCntr << "]: " << std::endl
                << std::flush;

        record_cntr = 0;
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

        for (int j = 0; j < state.xEkfPrev.cols(); j++) //TODO put this test to function!!!
        {
            for (int i = 0; i < state.xEkfPrev.rows(); i++)
            {
                state.xEkfPrev(i, j) = record[record_cntr++];
            }
        }

        for (int j = 0; j < state.pEkfPrev.cols(); j++) //TODO put this test to function!!!
        {
            for (int i = 0; i < state.pEkfPrev.rows(); i++)
            {
                state.pEkfPrev(i, j) = record[record_cntr++];
            }
        }

        for (int j = 0; j < ssMeas.cols(); j++) //TODO put this test to function!!!
        {
            for (int i = 0; i < ssMeas.rows(); i++)
            {
                ssMeas(i, j) = record[record_cntr++];
            }
        }

        for (int j = 0; j < gyrMeas.cols(); j++) //TODO put this test to function!!!
        {
            for (int i = 0; i < gyrMeas.rows(); i++)
            {
                gyrMeas(i, j) = record[record_cntr++];
            }
        }

        ssFlag = record[record_cntr++];
        gyrFlag = record[record_cntr++];

        for (int j = 0; j < state.ctrlTorquePrev.cols(); j++) //TODO put this test to function!!!
        {
            for (int i = 0; i < state.ctrlTorquePrev.rows(); i++)
            {
                state.ctrlTorquePrev(i, j) = record[record_cntr++];
            }
        }

        for (int j = 0; j < xPrio_exp.cols(); j++)
        {
            for (int i = 0; i < xPrio_exp.rows(); i++)
            {
                xPrio_exp(i, j) = record[record_cntr++];
            }
        }

        for (int j = 0; j < sv_SS_exp.cols(); j++)
        {
            for (int i = 0; i < sv_SS_exp.rows(); i++)
            {
                sv_SS_exp(i, j) = record[record_cntr++];
            }
        }

        for (int j = 0; j < dLong_dSS_exp.cols(); j++)
        {
            for (int i = 0; i < dLong_dSS_exp.rows(); i++)
            {
                dLong_dSS_exp(i, j) = record[record_cntr++];
            }
        }

        for (int j = 0; j < angrate_ss_exp.cols(); j++)
        {
            for (int i = 0; i < angrate_ss_exp.rows(); i++)
            {
                angrate_ss_exp(i, j) = record[record_cntr++];
            }
        }

        for (int j = 0; j < jacobianF_exp.cols(); j++)
        {
            for (int i = 0; i < jacobianF_exp.rows(); i++)
            {
                jacobianF_exp(i, j) = record[record_cntr++];
            }
        }

        for (int j = 0; j < P_prio_exp.cols(); j++)
        {
            for (int i = 0; i < P_prio_exp.rows(); i++)
            {
                P_prio_exp(i, j) = record[record_cntr++];
            }
        }

        for (int j = 0; j < innovCov_exp.cols(); j++)
        {
            for (int i = 0; i < innovCov_exp.rows(); i++)
            {
                innovCov_exp(i, j) = record[record_cntr++];
            }
        }

        for (int j = 0; j < innovCovInv_exp.cols(); j++)
        {
            for (int i = 0; i < innovCovInv_exp.rows(); i++)
            {
                innovCovInv_exp(i, j) = record[record_cntr++];
            }
        }

        for (int j = 0; j < K_exp.cols(); j++)
        {
            for (int i = 0; i < K_exp.rows(); i++)
            {
                K_exp(i, j) = record[record_cntr++];
            }
        }

        for (int j = 0; j < innov_exp.cols(); j++)
        {
            for (int i = 0; i < innov_exp.rows(); i++)
            {
                innov_exp(i, j) = record[record_cntr++];
            }
        }

        for (int j = 0; j < xEkf_exp.cols(); j++)
        {
            for (int i = 0; i < xEkf_exp.rows(); i++)
            {
                xEkf_exp(i, j) = record[record_cntr++];
            }
        }

        for (int j = 0; j < jf_m_exp.cols(); j++)
        {
            for (int i = 0; i < jf_m_exp.rows(); i++)
            {
                jf_m_exp(i, j) = record[record_cntr++];
            }
        }

        for (int j = 0; j < pEkf_exp.cols(); j++)
        {
            for (int i = 0; i < pEkf_exp.rows(); i++)
            {
                pEkf_exp(i, j) = record[record_cntr++];
            }
        }

        //---------vvv-----------ALGO--------------------vvv-------------------

        Matrix3f rotSS = Matrix3f(state.params.rotSS.data());
        Matrix3f rotSSt = Matrix3f(state.params.rotSS.data()).transpose();

        // PREDICTION -------------------------------------------------------------
        // Propagate state vector using RK4
        Vector5f k1 = PropagateState(state.xEkfPrev, state.ctrlTorquePrev,
                state);
        Vector5f k2 = PropagateState(
                state.xEkfPrev + 0.5f * state.params.dt * k1,
                state.ctrlTorquePrev, state);
        Vector5f k3 = PropagateState(
                state.xEkfPrev + 0.5f * state.params.dt * k2,
                state.ctrlTorquePrev, state);
        Vector5f k4 = PropagateState(state.xEkfPrev + state.params.dt * k3,
                state.ctrlTorquePrev, state);
        Vector5f xPrio = state.xEkfPrev
                + state.params.dt / 6.0f * (k1 + 2.0f * k2 + 2.0f * k3 + k4);

        // Extract previous states
        float ssLat = state.xEkfPrev(0, 0);
        float ssLong = state.xEkfPrev(1, 0);
        Vector3f angrate = state.xEkfPrev.block(2, 0, 3, 1);

        // Sun Vector in SS frame
        float sinLat = sinf(ssLat);
        float sinLong = sinf(ssLong);
        float cosLat = cosf(ssLat);
        float cosLong = cosf(ssLong);

        float sx = sinLat * cosLong;
        float sy = sinLat * sinLong;
        float sz = cosLat;
        Vector3f sv_SS;
        sv_SS << sx, sy, sz;

        // Partial Derivatives
        float szFactor = sqrtf(1.0f - powf(sz, 2.0f));
        RowVector3f dLat_dSS;
        dLat_dSS << 0.0f, 0.0f, -1.0f / szFactor;
        float htemp = sy / (szFactor + sx);
        float hx = -sy / powf(szFactor + sx, 2.0f);
        float hy = 1.0f / (szFactor + sx);
        float hz = -hx * sz / szFactor;
        RowVector3f dLong_dSS = 2.0f / (powf(htemp, 2.0f) + 1.0f)
                * (RowVector3f() << hx, hy, hz).finished();

        // Angular rate in SS frame
        Matrix3f angrate_ss = rotSS * skew(angrate) * rotSSt;

        // Sun vector in SS partial derivatives
        Vector3f dss_dLat;
        dss_dLat << cosLat * cosLong,              //
        cosLat * sinLong,              //
        -sinLat;
        Vector3f dss_dLong;
        dss_dLong << -sinLat * sinLong,              //
        sinLat * cosLong,              //
        0.0f;

        // Second order partial derivatives (row vectors)
        float sinLat2 = powf(sinLat, 2.0f);
        float tanHalfLong = tanf(ssLong / 2.0f);
        float tanHalfLong2 = powf(tanHalfLong, 2.0f);
        RowVector3f ddLat_dLat_dSS;
        ddLat_dLat_dSS << 0, 0, cosLat / sinLat2; //XXX is this init the same as above?? Vector3f ddLat_dLat_dSS({ 0, 0, cosf(ssLat) / powf(sinf(ssLat), 2) });
        RowVector3f ddLong_dLat_dSS = RowVector3f::Zero();
        RowVector3f ddLat_dLong_dSS;
        ddLat_dLong_dSS << tanHalfLong * cosLat / sinLat2, //
        -cosLat / sinLat2, //
        -tanHalfLong / sinLat * (2.0f / sinLat2 - 1.0f);
        RowVector3f ddLong_dLong_dSS;
        ddLong_dLong_dSS << -0.5f / sinLat * (1.0f + tanHalfLong2), //
        0.0f, //
        0.5f * cosLat / sinLat2 * (1.0f + tanHalfLong2);

        // Derivatives of the Sun angles time derivatives wrt the state vector elements
        Matrix3f skewRotSStSvSS = skew(rotSSt * sv_SS);
        float dLatDot_dLat = (-ddLat_dLat_dSS * angrate_ss * sv_SS
                - dLat_dSS * angrate_ss * dss_dLat).value();
        float dLatDot_dLong = (-ddLong_dLat_dSS * angrate_ss * sv_SS
                - dLat_dSS * angrate_ss * dss_dLong).value();
        RowVector3f dLatDot_dOmega = dLat_dSS * rotSS * skewRotSStSvSS;
        float dLongDot_dLat = (-ddLat_dLong_dSS * angrate_ss * sv_SS
                - dLong_dSS * angrate_ss * dss_dLat).value();
        float dLongDot_dLong = (-ddLong_dLong_dSS * angrate_ss * sv_SS
                - dLong_dSS * angrate_ss * dss_dLong).value();
        RowVector3f dLongDot_dOmega = dLong_dSS * rotSS * skewRotSStSvSS;

        // Derivative of the omega time derivative wrt omega
        Matrix3f dOmegaDot_dOmega =
                Matrix3f(state.params.inertiaInv.data())
                        * (skew(Matrix3f(state.params.inertia.data()) * angrate)
                                - skew(angrate)
                                        * Matrix3f(state.params.inertia.data()));

        // Discrete Jacobian for state transition
        Matrix5f jacobianF;
        jacobianF << dLatDot_dLat, dLatDot_dLong, dLatDot_dOmega, dLongDot_dLat, dLongDot_dLong, //XXX !!!!!! investigate
        dLongDot_dOmega, Matrix<float, 3, 2>::Zero(), dOmegaDot_dOmega;
//
//            Matrix5f jacobianFa;
//            jacobianFa << 0.1, 0.2, (RowVector3f() << 0.3,0.4,0.5).finished(), 0.6,0.7,(RowVector3f() << 0.8,0.9,0.01).finished(), Matrix<float, 3, 2>::Zero(), (Matrix3f() << 0.11,0.12,0.13,0.14,0.15,0.16,0.17,0.18,0.19).finished();
//            jacobianF.block(0, 0, 1, 1) = 0.1;
//            jacobianF.block(0, 1, 1, 1) = 0.2;
//            jacobianF.block(0, 2, 1, 3) = (Vector3f() << 0.3,0.4,0.5).finished();
//            jacobianF.block(2, 2, 3, 3) = dOmegaDot_dOmega;

//            std::cout<<jacobianF<<std::endl;
//            std::cout<<"drugi"<<std::endl;
//            std::cout<<jacobianF_exp<<std::endl;
//
        //std::cout << jacobianF<<std::endl;

        Matrix5f stateTrans = Matrix5f::Identity()
                + state.params.dt * jacobianF;
        // Prediction of covariance matrix P
        Matrix5f P_prio1 = stateTrans * state.pEkfPrev * stateTrans.transpose();
        Matrix5f P_prio2 = state.params.dt
                * Vector5f(state.params.kalmanCov.Q.data()).asDiagonal(); //XXX !!!!!! investigate
        Matrix5f P_prio = P_prio1 + P_prio2;

        // UPDATE -----------------------------------------------------------------
        // Predict and get sun sensor and gyro measurements
        Vector2f ssPrio = xPrio.block(0, 0, 2, 1);
        Vector3f gyroPrio = xPrio.block(2, 0, 3, 1);

        Vector5f zPrio;
        zPrio << ssPrio, gyroPrio;

        Vector2f ssMeasTmp = ssMeas;
        Vector3f gyroMeasTmp = gyrMeas;

        if (!ssFlag)
        {
            ssMeasTmp = ssPrio;
        }

        if (!gyrFlag)
        {
            gyroMeasTmp = gyroPrio;
        }

        Vector5f zMeas;
        zMeas << ssMeasTmp, gyroMeasTmp;

        // Kalman Gain
        innovCov = P_prio
                + Matrix5f(
                        Vector5f(state.params.kalmanCov.R.data()).asDiagonal());
        Matrix5f K = P_prio * matInv(innovCov);

        if (!ssFlag)
        {
            K.block(0, 0, 5, 2) = Matrix<float, 5, 2>::Zero();
        }

        if (!gyrFlag)
        {
            K.block(0, 2, 5, 3) = Matrix<float, 5, 3>::Zero();
        }

        // Innovation
        innov = zMeas - zPrio;
        Vector5f delta_x = K * innov;

        // Calculate outputs
        xEkf = xPrio + delta_x;
        Matrix5f jf_m = Matrix5f::Identity() - K;
        pEkf = jf_m * P_prio * jf_m.transpose()
                + K * Vector5f(state.params.kalmanCov.R.data()).asDiagonal()
                        * K.transpose(); // Joseph form

                        //---------^^^-----------ALGO--------------------^^^-------------------

        for (int j = 0; j < xPrio_exp.cols(); j++)
        {
            for (int i = 0; i < xPrio_exp.rows(); i++)
            {
                EXPECT_NEAR(xPrio_exp(i, j), xPrio(i, j), COMP_ACCUR);
#ifdef ADCS_SUNPOINTING_DEBUG
                std::cout << "EXPECT_NEAR(" << xPrio_exp(i, j) << " == "
                << xPrio(i, j) << ")" << std::endl;
#endif

            }
        }

        for (int j = 0; j < sv_SS_exp.cols(); j++)
        {
            for (int i = 0; i < sv_SS_exp.rows(); i++)
            {
                EXPECT_NEAR(sv_SS_exp(i, j), sv_SS(i, j), COMP_ACCUR);
#ifdef ADCS_SUNPOINTING_DEBUG
                std::cout << "EXPECT_NEAR(" << sv_SS_exp(i, j) << " == "
                << sv_SS(i, j) << ")" << std::endl;
#endif

            }
        }

        for (int j = 0; j < dLong_dSS_exp.cols(); j++)
        {
            for (int i = 0; i < dLong_dSS_exp.rows(); i++)
            {
                EXPECT_NEAR(dLong_dSS_exp(i, j), dLong_dSS(i, j), COMP_ACCUR);
#ifdef ADCS_SUNPOINTING_DEBUG
                std::cout << "EXPECT_NEAR(" << dLong_dSS_exp(i, j) << " == "
                << dLong_dSS(i, j) << ")" << std::endl;
#endif

            }
        }

        for (int j = 0; j < angrate_ss_exp.cols(); j++)
        {
            for (int i = 0; i < angrate_ss_exp.rows(); i++)
            {
                EXPECT_NEAR(angrate_ss_exp(i, j), angrate_ss(i, j), COMP_ACCUR);
#ifdef ADCS_SUNPOINTING_DEBUG
                std::cout << "EXPECT_NEAR(" << angrate_ss_exp(i, j) << " == "
                << angrate_ss(i, j) << ")" << std::endl;
#endif

            }
        }

        for (int j = 0; j < jacobianF_exp.cols(); j++)
        {
            for (int i = 0; i < jacobianF_exp.rows(); i++)
            {
                EXPECT_NEAR(jacobianF_exp(i, j), jacobianF(i, j), COMP_ACCUR);
#ifdef ADCS_SUNPOINTING_DEBUG
                std::cout << "EXPECT_NEAR(" << jacobianF_exp(i, j) << " == "
                << jacobianF(i, j) << ")" << std::endl;
#endif

            }
        }

        for (int j = 0; j < P_prio_exp.cols(); j++)
        {
            for (int i = 0; i < P_prio_exp.rows(); i++)
            {
                EXPECT_NEAR(P_prio_exp(i, j), P_prio(i, j), COMP_ACCUR);
#ifdef ADCS_SUNPOINTING_DEBUG
                std::cout << "EXPECT_NEAR(" << P_prio_exp(i, j) << " == "
                << P_prio(i, j) << ")" << std::endl;
#endif

            }
        }

        for (int j = 0; j < innovCov_exp.cols(); j++)
        {
            for (int i = 0; i < innovCov_exp.rows(); i++)
            {
                EXPECT_NEAR(innovCov_exp(i, j), innovCov(i, j), COMP_ACCUR);
#ifdef ADCS_SUNPOINTING_DEBUG
                std::cout << "EXPECT_NEAR(" << innovCov_exp(i, j) << " == "
                << innovCov(i, j) << ")" << std::endl;
#endif

            }
        }

//            for (int j = 0; j < innovCovInv_exp.cols(); j++)
//            {
//                for (int i = 0; i < innovCovInv_exp.rows(); i++)
//                {
//                    EXPECT_NEAR(innovCovInv_exp(i, j), innovCovInv(i, j),
//                            COMP_ACCUR);
//#ifdef ADCS_SUNPOINTING_DEBUG
//                    std::cout << "EXPECT_NEAR(" << innovCovInv_exp(i, j) << " == "
//                    << innovCovInv(i, j) << ")" << std::endl;
//#endif
//
//                }
//            }

        for (int j = 0; j < K_exp.cols(); j++)
        {
            for (int i = 0; i < K_exp.rows(); i++)
            {
                EXPECT_NEAR(K_exp(i, j), K(i, j), COMP_ACCUR);
#ifdef ADCS_SUNPOINTING_DEBUG
                std::cout << "EXPECT_NEAR(" << K_exp(i, j) << " == "
                << K(i, j) << ")" << std::endl;
#endif

            }
        }

        for (int j = 0; j < innov_exp.cols(); j++)
        {
            for (int i = 0; i < innov_exp.rows(); i++)
            {
                EXPECT_NEAR(innov_exp(i, j), innov(i, j), COMP_ACCUR);
#ifdef ADCS_SUNPOINTING_DEBUG
                std::cout << "EXPECT_NEAR(" << innov_exp(i, j) << " == "
                << innov(i, j) << ")" << std::endl;
#endif

            }
        }

        for (int j = 0; j < xEkf_exp.cols(); j++)
        {
            for (int i = 0; i < xEkf_exp.rows(); i++)
            {
                EXPECT_NEAR(xEkf_exp(i, j), xEkf(i, j), COMP_ACCUR);
#ifdef ADCS_SUNPOINTING_DEBUG
                std::cout << "EXPECT_NEAR(" << xEkf_exp(i, j) << " == "
                << xEkf(i, j) << ")" << std::endl;
#endif

            }
        }

        for (int j = 0; j < jf_m_exp.cols(); j++)
        {
            for (int i = 0; i < jf_m_exp.rows(); i++)
            {
                EXPECT_NEAR(jf_m_exp(i, j), jf_m(i, j), COMP_ACCUR);
#ifdef ADCS_SUNPOINTING_DEBUG
                std::cout << "EXPECT_NEAR(" << jf_m_exp(i, j) << " == "
                << jf_m(i, j) << ")" << std::endl;
#endif

            }
        }

        for (int j = 0; j < pEkf_exp.cols(); j++)
        {
            for (int i = 0; i < pEkf_exp.rows(); i++)
            {
                EXPECT_NEAR(pEkf_exp(i, j), pEkf(i, j), COMP_ACCUR);
#ifdef ADCS_SUNPOINTING_DEBUG
                std::cout << "EXPECT_NEAR(" << pEkf_exp(i, j) << " == "
                << pEkf(i, j) << ")" << std::endl;
#endif

            }
        }
//*/
    }
    file.close();
}
//*/

// cross-validation of sunpointing against matlab implementation
TEST(sunpointing, cross_validation)
{
    //return;//XXX

    const float COMP_ACCUR = 1e-4;

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
    float mtm_input_scale = 1e7f;    //1e7f;
    // output: Sim [Am2] --> OBC [1e-4 Am2]
    float output_scale = 1e4f;

    int LoopCntr = -1;

    while (!file.eof())
    //for (int i = 0; i < 10; i++)
    {
        LoopCntr++;
        std::cout << "LoopCntr [" << LoopCntr << "]: " << std::endl
                << std::flush;

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
        std::cout << "in0: " << record[ESP_mtmMeas] << " " << mtm_input_scale << " " << record[ESP_mtmMeas] * mtm_input_scale << " " << (int)(record[1] * mtm_input_scale)
        << std::endl;
        std::cout << "in1: " << record[ESP_ssMeas] << " " << 1 << " " << record[ESP_ssMeas] * 1 << " " << (int)(record[2] * 1)
        << std::endl;
        std::cout << "in2: " << record[ESP_gyroMeas] << " " << 1 << " " << record[ESP_gyroMeas] * 1 << " " << (int)(record[3] * 1)
        << std::endl;
        std::cout << "out0: " << record[ESP_commDipoleSP] << " " << output_scale << " " << record[ESP_commDipoleSP] * output_scale << " "
        << (int)(record[ESP_commDipoleSP] * output_scale) << std::endl;
        std::cout << "out1: " << record[ESP_commDipoleSP+1] << " " << output_scale << " " << record[ESP_commDipoleSP+1] * output_scale << " "
        << (int)(record[ESP_commDipoleSP+1] * output_scale) << std::endl;
        std::cout << "out2: " << record[ESP_commDipoleSP+2] << " " << output_scale << " " << record[ESP_commDipoleSP+2] * output_scale << " "
        << (int)(record[ESP_commDipoleSP+2] * output_scale) << std::endl;
#else

#endif
        DipoleVec dipole =
        { 0, 0, 0 };
        MagVec mtmMeas =
        { int32_t(record[ESP_mtmMeas + 0] * mtm_input_scale), int32_t(
                record[ESP_mtmMeas + 1] * mtm_input_scale), int32_t(
                record[ESP_mtmMeas + 2] * mtm_input_scale) };
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

        EXPECT_NEAR(dipole[0] / output_scale, record[ESP_commDipoleSP + 0],
                COMP_ACCUR);
        EXPECT_NEAR(dipole[1] / output_scale, record[ESP_commDipoleSP + 1],
                COMP_ACCUR);
        EXPECT_NEAR(dipole[2] / output_scale, record[ESP_commDipoleSP + 2],
                COMP_ACCUR);

#ifdef ADCS_SUNPOINTING_DEBUG
        std::cout << "EXPECT_NEAR(" << dipole[0] / output_scale << " == "
        << record[ESP_commDipoleSP + 0] << ")" << std::endl;
        std::cout << "EXPECT_NEAR(" << dipole[1] / output_scale << " == "
        << record[ESP_commDipoleSP + 1] << ")" << std::endl;
        std::cout << "EXPECT_NEAR(" << dipole[2] / output_scale << " == "
        << record[ESP_commDipoleSP + 2] << ")" << std::endl;
#endif
    }
    file.close();
}
//*/
