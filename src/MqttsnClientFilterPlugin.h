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

#include "MqttsnClientFilter.h"

#include <cc_tools_qt/Plugin.h>

namespace cc_plugin_mqttsn_client_filter
{

class MqttsnClientFilterPlugin : public cc_tools_qt::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cc.MqttsnClientFilter" FILE "cc_plugin_mqttsn_client_filter.json")
    Q_INTERFACES(cc_tools_qt::Plugin)

public:
    MqttsnClientFilterPlugin();
    ~MqttsnClientFilterPlugin() noexcept;

protected:
    virtual void getCurrentConfigImpl(QVariantMap& config) override;
    virtual void reconfigureImpl(const QVariantMap& config) override;
    virtual void applyInterPluginConfigImpl(const QVariantMap& props) override;     

private:
    void createFilterIfNeeded();
    MqttsnClientFilterPtr m_filter;
};

}  // namespace cc_plugin_mqttsn_client_filter




