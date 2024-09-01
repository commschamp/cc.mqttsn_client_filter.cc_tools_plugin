//
// Copyright 2024 - 2025 (C). Alex Robenko. All rights reserved.
//

// This file is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


#include "MqttsnClientFilterSubConfigWidget.h"

#include <algorithm>

namespace cc_plugin_mqttsn_client_filter
{

MqttsnClientFilterSubConfigWidget::MqttsnClientFilterSubConfigWidget(MqttsnClientFilter& filter, SubConfig& config, QWidget* parentObj) : 
    Base(parentObj),
    m_filter(filter),
    m_config(config)
{
    m_ui.setupUi(this);

    m_ui.m_topicLineEdit->setText(m_config.m_topic);
    if (m_config.m_topic.isEmpty()) {
        m_ui.m_topicIdSpinBox->setValue(m_config.m_topicId);
    }
    m_ui.m_maxQosSpinBox->setValue(m_config.m_maxQos);
    refresh();

    connect(
        m_ui.m_topicLineEdit, &QLineEdit::textChanged,
        this, &MqttsnClientFilterSubConfigWidget::topicUpdated);   

    connect(
        m_ui.m_topicIdSpinBox, qOverload<int>(&QSpinBox::valueChanged),
        this, &MqttsnClientFilterSubConfigWidget::topicIdUpdated);          

    connect(
        m_ui.m_maxQosSpinBox, qOverload<int>(&QSpinBox::valueChanged),
        this, &MqttsnClientFilterSubConfigWidget::maxQosUpdated);  

    connect(
        m_ui.m_delToolButton, &QToolButton::clicked,
        this, &MqttsnClientFilterSubConfigWidget::delClicked);           
}

void MqttsnClientFilterSubConfigWidget::topicUpdated(const QString& val)
{
    m_config.m_topic = val;
    m_filter.forceCleanSession();
    refresh();
}

void MqttsnClientFilterSubConfigWidget::topicIdUpdated(int val)
{
    m_config.m_topicId = val;
    m_filter.forceCleanSession();
    refresh();
}

void MqttsnClientFilterSubConfigWidget::maxQosUpdated(int val)
{
    m_config.m_maxQos = val;
    m_filter.forceCleanSession();
}

void MqttsnClientFilterSubConfigWidget::delClicked([[maybe_unused]] bool checked)
{
    auto& subs = m_filter.config().m_subscribes;
    auto iter = 
        std::find_if(
            subs.begin(), subs.end(), 
            [this](auto& info)
            {
                return &m_config == &info;
            });

    if (iter == subs.end()) {
        assert(false); // should not happen
        return;
    }

    subs.erase(iter);
    m_filter.forceCleanSession();
    blockSignals(true);
    deleteLater();
}

void MqttsnClientFilterSubConfigWidget::refresh()
{
    bool useTopic = (!m_ui.m_topicLineEdit->text().isEmpty());
    if (useTopic) {
        m_ui.m_topicLineEdit->setEnabled(true);
        m_ui.m_topicIdSpinBox->setValue(0);
        m_ui.m_topicIdSpinBox->setEnabled(false);
        return;
    }

    bool useTopicId = (m_ui.m_topicIdSpinBox->value() != 0);
    if (useTopicId) {
        m_ui.m_topicIdSpinBox->setEnabled(true);
        m_ui.m_topicLineEdit->clear();
        m_ui.m_topicLineEdit->setEnabled(false);
        return;
    }

    m_ui.m_topicLineEdit->setEnabled(true);
    m_ui.m_topicIdSpinBox->setEnabled(true);
}


}  // namespace cc_plugin_mqttsn_client_filter


