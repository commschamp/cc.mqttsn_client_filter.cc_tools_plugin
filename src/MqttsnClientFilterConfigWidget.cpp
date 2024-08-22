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

#include "MqttsnClientFilterConfigWidget.h"

#include "MqttsnClientFilterSubConfigWidget.h"

#include <cassert>

#include <QtCore/QtGlobal>

namespace cc_plugin_mqttsn_client_filter
{

namespace 
{

void deleteAllWidgetsFrom(QLayout& layout)
{
    while (true) {
        auto* child = layout.takeAt(0);
        if (child == nullptr) {
            break;
        }

        delete child->widget();
        delete child;
    }
}

} // namespace 
    

MqttsnClientFilterConfigWidget::MqttsnClientFilterConfigWidget(MqttsnClientFilter& filter, QWidget* parentObj) :
    Base(parentObj),
    m_filter(filter)
{
    m_ui.setupUi(this);

    auto subsLayout = new QVBoxLayout;
    m_ui.m_subsWidget->setLayout(subsLayout);

    refresh();

    connect(
        &m_filter, &MqttsnClientFilter::sigConfigChanged,
        this, &MqttsnClientFilterConfigWidget::refresh);     

    connect(
        m_ui.m_retryPeriodSpinBox, qOverload<int>(&QSpinBox::valueChanged),
        this, &MqttsnClientFilterConfigWidget::retryPeriodUpdated); 

    connect(
        m_ui.m_retryCountSpinBox, qOverload<int>(&QSpinBox::valueChanged),
        this, &MqttsnClientFilterConfigWidget::retryCountUpdated);             

    connect(
        m_ui.m_clientIdLineEdit, &QLineEdit::textChanged,
        this, &MqttsnClientFilterConfigWidget::clientIdUpdated);

    connect(
        m_ui.m_keepAliveSpinBox, qOverload<int>(&QSpinBox::valueChanged),
        this, &MqttsnClientFilterConfigWidget::keepAliveUpdated);    

    connect(
        m_ui.m_cleanSessionComboBox, qOverload<int>(&QComboBox::currentIndexChanged),
        this, &MqttsnClientFilterConfigWidget::forcedCleanSessionUpdated);           

    connect(
        m_ui.m_pubTopicLineEdit, &QLineEdit::textChanged,
        this, &MqttsnClientFilterConfigWidget::pubTopicUpdated);       

    connect(
        m_ui.m_pubTopicIdSpinBox, qOverload<int>(&QSpinBox::valueChanged),
        this, &MqttsnClientFilterConfigWidget::pubTopicIdUpdated);           

    connect(
        m_ui.m_pubQosSpinBox, qOverload<int>(&QSpinBox::valueChanged),
        this, &MqttsnClientFilterConfigWidget::pubQosUpdated);   

    connect(
        m_ui.m_addSubPushButton, &QPushButton::clicked,
        this, &MqttsnClientFilterConfigWidget::addSubscribe);           
}

MqttsnClientFilterConfigWidget::~MqttsnClientFilterConfigWidget() noexcept = default;

void MqttsnClientFilterConfigWidget::refresh()
{
    deleteAllWidgetsFrom(*(m_ui.m_subsWidget->layout()));

    for (auto& subConfig : m_filter.config().m_subscribes) {
        addSubscribeWidget(subConfig);
    }    

    m_ui.m_retryPeriodSpinBox->setValue(m_filter.config().m_retryPeriod);
    m_ui.m_retryCountSpinBox->setValue(m_filter.config().m_retryCount);
    m_ui.m_clientIdLineEdit->setText(m_filter.config().m_clientId);
    m_ui.m_keepAliveSpinBox->setValue(static_cast<int>(m_filter.config().m_keepAlive));
    m_ui.m_cleanSessionComboBox->setCurrentIndex(static_cast<int>(m_filter.config().m_forcedCleanSession));
    m_ui.m_pubTopicLineEdit->setText(m_filter.config().m_pubTopic);
    m_ui.m_pubTopicIdSpinBox->setValue(m_filter.config().m_pubTopicId);
    m_ui.m_pubQosSpinBox->setValue(m_filter.config().m_pubQos);

    refreshSubscribes();
}

void MqttsnClientFilterConfigWidget::retryPeriodUpdated(int val)
{
    m_filter.config().m_retryPeriod = static_cast<unsigned>(val);
}

void MqttsnClientFilterConfigWidget::retryCountUpdated(int val)
{
    m_filter.config().m_retryCount = static_cast<unsigned>(val);
}

void MqttsnClientFilterConfigWidget::clientIdUpdated(const QString& val)
{
    if (m_filter.config().m_clientId == val) {
        return;
    }

    m_filter.config().m_clientId = val;
    m_filter.forceCleanSession();
}

void MqttsnClientFilterConfigWidget::keepAliveUpdated(int val)
{
    m_filter.config().m_keepAlive = static_cast<unsigned>(val);
}

void MqttsnClientFilterConfigWidget::forcedCleanSessionUpdated(int val)
{
    m_filter.config().m_forcedCleanSession = (val > 0);
}

void MqttsnClientFilterConfigWidget::pubTopicUpdated(const QString& val)
{
    m_filter.config().m_pubTopic = val;
}

void MqttsnClientFilterConfigWidget::pubTopicIdUpdated(int val)
{
    m_filter.config().m_pubTopicId = static_cast<unsigned>(val);
}

void MqttsnClientFilterConfigWidget::pubQosUpdated(int val)
{
    m_filter.config().m_pubQos = val;
}

void MqttsnClientFilterConfigWidget::addSubscribe()
{
    auto& subs = m_filter.config().m_subscribes;
    subs.resize(subs.size() + 1U);
    addSubscribeWidget(subs.back());
    refreshSubscribes();
}

void MqttsnClientFilterConfigWidget::refreshSubscribes()
{
    bool subscribesVisible = !m_filter.config().m_subscribes.empty();
    m_ui.m_subsWidget->setVisible(subscribesVisible);
}

void MqttsnClientFilterConfigWidget::addSubscribeWidget(SubConfig& config)
{
    auto* widget = new MqttsnClientFilterSubConfigWidget(m_filter, config, this);
    connect(
        widget, &QObject::destroyed,
        this,
        [this](QObject*)
        {
            refreshSubscribes();
        });

    auto* subsLayout = qobject_cast<QVBoxLayout*>(m_ui.m_subsWidget->layout());
    assert(subsLayout != nullptr);
    subsLayout->addWidget(widget); 
}

}  // namespace cc_plugin_mqttsn_client_filter


