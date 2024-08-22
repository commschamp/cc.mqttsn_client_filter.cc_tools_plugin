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


#pragma once

#include "ui_MqttsnClientFilterSubConfigWidget.h"

#include "MqttsnClientFilter.h"

#include <QtWidgets/QWidget>


namespace cc_plugin_mqttsn_client_filter
{

class MqttsnClientFilterSubConfigWidget : public QWidget
{
    Q_OBJECT
    using Base = QWidget;

public:
    using SubConfig = MqttsnClientFilter::SubConfig;

    explicit MqttsnClientFilterSubConfigWidget(MqttsnClientFilter& filter, SubConfig& config, QWidget* parentObj = nullptr);
    ~MqttsnClientFilterSubConfigWidget() noexcept = default;

private slots:
    void topicUpdated(const QString& val);
    void maxQosUpdated(int val);
    void delClicked(bool checked);

private:
    MqttsnClientFilter& m_filter;
    SubConfig& m_config;
    Ui::MqttsnClientFilterSubConfigWidget m_ui;
};

}  // namespace cc_plugin_mqttsn_client_filter


