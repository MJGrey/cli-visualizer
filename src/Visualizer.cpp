/*
 * Visualizer.cpp
 *
 * Created on: Jul 30, 2015
 *     Author: dpayne
 */

#include "Visualizer.h"
#include "Source/MpdAudioSource.h"
#include "Domain/VisConstants.h"
#include "Domain/VisException.h"
#include "Transformer/SpectrumTransformer.h"

#include <iostream>

vis::Visualizer::Visualizer(const vis::Settings *const settings)
    : m_current_audio_source{nullptr}, m_current_transformer{nullptr},
      m_shutdown{false}, m_settings{settings}, m_pcm_buffer{nullptr}
{
    m_pcm_buffer = static_cast<pcm_stereo_sample *>(
        calloc(m_settings->get_sample_size(), sizeof(pcm_stereo_sample)));
}

void vis::Visualizer::add_audio_source(const std::string &audio_source)
{
    if (audio_source == VisConstants::kMpdAudioSourceName)
    {
        m_audio_sources.emplace_back(std::unique_ptr<vis::MpdAudioSource>{
            new vis::MpdAudioSource{m_settings}});
    }
}

void vis::Visualizer::run()
{
    setup_audio_sources();

    setup_transformers();

    m_current_audio_source = m_audio_sources[0].get();

    AudioSource *audioSource = get_current_audio_source();
    GenericTransformer *transformer = get_current_transformer();
    while (!should_shutdown() &&
           audioSource->read(m_pcm_buffer, m_settings->get_sample_size()))
    {
        transformer->execute(m_pcm_buffer);
        audioSource = get_current_audio_source();
        transformer = get_current_transformer();
    }
}

void vis::Visualizer::setup_audio_sources()
{
    for (const auto &audioSource : m_settings->get_audio_sources())
    {
        add_audio_source(audioSource);
    }

    // Throw an error if there are no audio sources
    if (m_audio_sources.size() < 1)
    {
        throw vis::VisException{"No audio sources defined"};
    }
}

void vis::Visualizer::setup_transformers()
{
    m_transformers.emplace_back(std::unique_ptr<vis::SpectrumTransformer>{
        new vis::SpectrumTransformer{m_settings}});
}

vis::Visualizer::~Visualizer()
{
    if (m_pcm_buffer != nullptr)
    {
        free(m_pcm_buffer);
    }
}
