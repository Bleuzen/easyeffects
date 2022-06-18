/*
 *  Copyright © 2017-2022 Wellington Wallace
 *
 *  This file is part of EasyEffects.
 *
 *  EasyEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  EasyEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with EasyEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "lv2_wrapper.hpp"
#include "plugin_base.hpp"

class Limiter : public PluginBase {
 public:
  Limiter(const std::string& tag, const std::string& schema, const std::string& schema_path, PipeManager* pipe_manager);
  Limiter(const Limiter&) = delete;
  auto operator=(const Limiter&) -> Limiter& = delete;
  Limiter(const Limiter&&) = delete;
  auto operator=(const Limiter&&) -> Limiter& = delete;
  ~Limiter() override;

  void setup() override;

  void process(std::span<float>& left_in,
               std::span<float>& right_in,
               std::span<float>& left_out,
               std::span<float>& right_out,
               std::span<float>& probe_left,
               std::span<float>& probe_right) override;

  void update_probe_links() override;

  auto get_latency_seconds() -> float override;

  sigc::signal<void(const float&)> gain_left, gain_right, sidechain_left, sidechain_right, latency;

  float gain_l_port_value = 0.0F;
  float gain_r_port_value = 0.0F;
  float sidechain_l_port_value = 0.0F;
  float sidechain_r_port_value = 0.0F;
  float latency_port_value = 0.0F;

 private:
  uint latency_n_frames = 0U;

  std::unique_ptr<lv2::Lv2Wrapper> lv2_wrapper;

  std::vector<pw_proxy*> list_proxies;

  void update_sidechain_links(const std::string& key);
};
