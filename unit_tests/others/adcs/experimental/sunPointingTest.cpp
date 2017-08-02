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

#define COMP_ACCUR 0.01

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
/*
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
    const uint recordLen = 50;   //in: 5x5 + out: 5x5
    Matrix5f in;
    Matrix5f res;
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

        for (int j = 0; j < in.cols(); j++)
        {
            for (int i = 0; i < in.rows(); i++)
            {
                in(i, j) = record[i + j * in.cols()];
            }
        }
        res = matInv(in);   // accuracy problems with original method
        //res = in.inverse();   // perfect result

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
TEST(sunpointing, cross_validation_sunVandRateExtendedInDeep)
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
                EXPECT_NEAR(SunVector_SS(i, j), record[record_cntr], 0.0001);
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

// cross-validation of sunpointing against matlab implementation
TEST(sunpointing, cross_validation)
{
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

    while (!file.eof())
    //for (int i = 0; i < 10; i++)
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
        { int32_t(record[ESP_mtmMeas + 0] * mtm_input_scale), int32_t(record[ESP_mtmMeas + 1] * mtm_input_scale),
                int32_t(record[ESP_mtmMeas + 2] * mtm_input_scale) };
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
                10);
        EXPECT_NEAR(dipole[1], record[ESP_commDipoleSP + 1] * output_scale,
                10);
        EXPECT_NEAR(dipole[2], record[ESP_commDipoleSP + 2] * output_scale,
                10);

        //#ifdef ADCS_SUNPOINTING_DEBUG
        std::cout << "EXPECT_NEAR(" << dipole[0] << " == "
                << record[ESP_commDipoleSP + 0] * output_scale << ")"
                << std::endl;
        std::cout << "EXPECT_NEAR(" << dipole[1] << " == "
                << record[ESP_commDipoleSP + 1] * output_scale << ")"
                << std::endl;
        std::cout << "EXPECT_NEAR(" << dipole[2] << " == "
                << record[ESP_commDipoleSP + 2] * output_scale << ")"
                << std::endl;
        //#endif
    }
    file.close();
}
//*/
