/*
 * statistics.c
 */

#include "statistics.h"
#include "atomic_vars.h"

static QByteArray rs_statistics_queue(RS_STATISTICS_QUEUE_SIZE, 0);     // Queue for RS statistics, consists of 'RS_STATISTICS_QUEUE_SIZE' elements.
                                                                        // Queue element = 1 means successful RS correction, 0 - no correction was needed (frame with good crc)
static uint8_t rs_cnt = 0;

/*
 * @brief Reset statistics
*/
void statisics_reset()
{
    m_rxHighSpeedStatistics = 0;

    m_ReedSolomonCorrectionsCounter = 0;
    m_QamFramesCounter = 0;
    m_CrcErrorsCounter = 0;

    for(uint8_t i = 0; i < RS_STATISTICS_QUEUE_SIZE; ++i)
        rs_statistics_queue[i] = 0;
}

/*
 * @brief Increments m_rxHighSpeedStatistics when good crc packet (QAM frame) is received
*/
void crc_statistics_good_crc_received()
{
    if (m_rxHighSpeedStatistics < CRC_MAX_STATISTICS)
    {
        ++m_rxHighSpeedStatistics;
    }
}

/*
 * @brief Decrements m_rxHighSpeedStatistics when bad crc packet (QAM frame) is received
*/
void crc_statistics_bad_crc_received()
{
    ++m_CrcErrorsCounter;

    if (m_rxHighSpeedStatistics > 0)
    {
        --m_rxHighSpeedStatistics;
    }
}

/*
 * @brief Counts successful RS corrections
*/
void rs_statistics_add_correction()
{
    ++m_ReedSolomonCorrectionsCounter;

    rs_statistics_queue[rs_cnt] = 1;
    if(++rs_cnt == RS_STATISTICS_QUEUE_SIZE)
        rs_cnt = 0;
}

/*
 * @brief Counts good packets (QAM frames) with no RS corrections needed
*/
void rs_statistics_add_no_correction()
{
    rs_statistics_queue[rs_cnt] = 0;
    if(++rs_cnt == RS_STATISTICS_QUEUE_SIZE)
        rs_cnt = 0;
}

/*
 * @brief Calculates RS statistics: how many successful corrections were made
 * in the last 'RS_STATISTICS_QUEUE_SIZE' packets (QAM frames)
*/
uint8_t rs_calculate_statistics()
{
    uint8_t rs_statistics = 0;

    for(uint8_t i = 0; i < RS_STATISTICS_QUEUE_SIZE; ++i)
        rs_statistics += rs_statistics_queue[i];

    return rs_statistics;
}
