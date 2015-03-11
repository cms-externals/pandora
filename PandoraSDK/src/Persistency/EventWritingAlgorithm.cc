/**
 *  @file   PandoraSDK/src/Persistency/EventWritingAlgorithm.cc
 * 
 *  @brief  Implementation of the event writing algorithm class.
 * 
 *  $Log: $
 */

#include "Pandora/AlgorithmHeaders.h"

#include "Persistency/EventWritingAlgorithm.h"
#include "Persistency/BinaryFileWriter.h"
#include "Persistency/XmlFileWriter.h"

using namespace pandora;

EventWritingAlgorithm::EventWritingAlgorithm() :
    m_geometryFileType(UNKNOWN_FILE_TYPE),
    m_eventFileType(UNKNOWN_FILE_TYPE),
    m_shouldWriteGeometry(false),
    m_shouldWriteEvents(true),
    m_shouldWriteMCRelationships(true),
    m_shouldWriteTrackRelationships(true),
    m_shouldOverwriteEventFile(false),
    m_shouldOverwriteGeometryFile(false),
    m_pEventFileWriter(NULL)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------

EventWritingAlgorithm::~EventWritingAlgorithm()
{
    delete m_pEventFileWriter;
}

//------------------------------------------------------------------------------------------------------------------------------------------

StatusCode EventWritingAlgorithm::Initialize()
{
    if (m_shouldWriteGeometry)
    {
        const FileMode fileMode(m_shouldOverwriteGeometryFile ? OVERWRITE : APPEND);

        if (BINARY == m_geometryFileType)
        {
            BinaryFileWriter geometryFileWriter(this->GetPandora(), m_geometryFileName, fileMode);
            PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, geometryFileWriter.WriteGeometry());
        }
        else if (XML == m_geometryFileType)
        {
            XmlFileWriter geometryFileWriter(this->GetPandora(), m_geometryFileName, fileMode);
            PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, geometryFileWriter.WriteGeometry());
        }
        else
        {
            return STATUS_CODE_FAILURE;
        }
    }

    if (m_shouldWriteEvents)
    {
        const FileMode fileMode(m_shouldOverwriteEventFile ? OVERWRITE : APPEND);

        if (BINARY == m_eventFileType)
        {
            m_pEventFileWriter = new BinaryFileWriter(this->GetPandora(), m_eventFileName, fileMode);
        }
        else if (XML == m_eventFileType)
        {
            m_pEventFileWriter = new XmlFileWriter(this->GetPandora(), m_eventFileName, fileMode);
        }
        else
        {
            return STATUS_CODE_FAILURE;
        }
    }

    return STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

StatusCode EventWritingAlgorithm::Run()
{
    if ((NULL != m_pEventFileWriter) && m_shouldWriteEvents)
    {
        const CaloHitList *pCaloHitList = NULL;
        PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pCaloHitList));

        const TrackList *pTrackList = NULL;
        PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pTrackList));

        const MCParticleList *pMCParticleList = NULL;
        PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pMCParticleList));

        PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, m_pEventFileWriter->WriteEvent(*pCaloHitList, *pTrackList, *pMCParticleList,
            m_shouldWriteMCRelationships, m_shouldWriteTrackRelationships));
    }

    return STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

StatusCode EventWritingAlgorithm::ReadSettings(const TiXmlHandle xmlHandle)
{
    PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle,
        "ShouldWriteGeometry", m_shouldWriteGeometry));

    if (m_shouldWriteGeometry)
    {
        PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, XmlHelper::ReadValue(xmlHandle,
            "GeometryFileName", m_geometryFileName));

        std::string fileExtension(m_geometryFileName.substr(m_geometryFileName.find_last_of(".")));
        std::transform(fileExtension.begin(), fileExtension.end(), fileExtension.begin(), ::tolower);

        if (std::string(".xml") == fileExtension)
        {
            m_geometryFileType = XML;
        }
        else if (std::string(".pndr") == fileExtension)
        {
            m_geometryFileType = BINARY;
        }
        else
        {
            std::cout << "EventReadingAlgorithm: Unknown geometry file type specified " << std::endl;
            return STATUS_CODE_INVALID_PARAMETER;
        }
    }

    PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle,
        "ShouldWriteEvents", m_shouldWriteEvents));

    if (m_shouldWriteEvents)
    {
        PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, XmlHelper::ReadValue(xmlHandle,
            "EventFileName", m_eventFileName));

        std::string fileExtension(m_eventFileName.substr(m_eventFileName.find_last_of(".")));
        std::transform(fileExtension.begin(), fileExtension.end(), fileExtension.begin(), ::tolower);

        if (std::string(".xml") == fileExtension)
        {
            m_eventFileType = XML;
        }
        else if (std::string(".pndr") == fileExtension)
        {
            m_eventFileType = BINARY;
        }
        else
        {
            std::cout << "EventReadingAlgorithm: Unknown event file type specified " << std::endl;
            return STATUS_CODE_INVALID_PARAMETER;
        }
    }

    PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle,
        "ShouldOverwriteEventFile", m_shouldOverwriteEventFile));

    PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle,
        "ShouldOverwriteGeometryFile", m_shouldOverwriteGeometryFile));

    PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle,
        "ShouldWriteMCRelationships", m_shouldWriteMCRelationships));

    PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle,
        "ShouldWriteTrackRelationships", m_shouldWriteTrackRelationships));

    return STATUS_CODE_SUCCESS;
}
