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

#include "equalizer_preset.hpp"

using namespace tags::equalizer;

EqualizerPreset::EqualizerPreset(PresetType preset_type, const int& index)
    : PluginPresetBase(tags::schema::equalizer::id,
                       tags::schema::equalizer::input_path,
                       tags::schema::equalizer::output_path,
                       preset_type,
                       index),
      input_settings_left(
          g_settings_new_with_path(tags::schema::equalizer::channel_id, tags::schema::equalizer::input_path_left)),
      input_settings_right(
          g_settings_new_with_path(tags::schema::equalizer::channel_id, tags::schema::equalizer::input_path_right)),
      output_settings_left(
          g_settings_new_with_path(tags::schema::equalizer::channel_id, tags::schema::equalizer::output_path_left)),
      output_settings_right(
          g_settings_new_with_path(tags::schema::equalizer::channel_id, tags::schema::equalizer::output_path_right)) {}

EqualizerPreset::~EqualizerPreset() {
  g_object_unref(input_settings_left);
  g_object_unref(input_settings_right);
  g_object_unref(output_settings_left);
  g_object_unref(output_settings_right);
}

void EqualizerPreset::save(nlohmann::json& json) {
  json[section]["equalizer"]["bypass"] = g_settings_get_boolean(settings, "bypass") != 0;

  json[section]["equalizer"]["input-gain"] = g_settings_get_double(settings, "input-gain");

  json[section]["equalizer"]["output-gain"] = g_settings_get_double(settings, "output-gain");

  json[section]["equalizer"]["mode"] = util::gsettings_get_string(settings, "mode");

  json[section]["equalizer"]["split-channels"] = g_settings_get_boolean(settings, "split-channels") != 0;

  json[section]["equalizer"]["balance"] = g_settings_get_double(settings, "balance");

  json[section]["equalizer"]["pitch-left"] = g_settings_get_double(settings, "pitch-left");

  json[section]["equalizer"]["pitch-right"] = g_settings_get_double(settings, "pitch-right");

  const auto nbands = g_settings_get_int(settings, "num-bands");

  json[section]["equalizer"]["num-bands"] = nbands;

  if (section == "input") {
    save_channel(json[section]["equalizer"]["left"], input_settings_left, nbands);
    save_channel(json[section]["equalizer"]["right"], input_settings_right, nbands);
  } else if (section == "output") {
    save_channel(json[section]["equalizer"]["left"], output_settings_left, nbands);
    save_channel(json[section]["equalizer"]["right"], output_settings_right, nbands);
  }
}

void EqualizerPreset::save_channel(nlohmann::json& json, GSettings* settings, const int& nbands) {
  for (int n = 0; n < nbands; n++) {
    const auto* const bandn = band_id[n];

    json[bandn]["type"] = util::gsettings_get_string(settings, band_type[n].data());

    json[bandn]["mode"] = util::gsettings_get_string(settings, band_mode[n].data());

    json[bandn]["slope"] = util::gsettings_get_string(settings, band_slope[n].data());

    json[bandn]["solo"] = g_settings_get_boolean(settings, band_solo[n].data()) != 0;

    json[bandn]["mute"] = g_settings_get_boolean(settings, band_mute[n].data()) != 0;

    json[bandn]["gain"] = g_settings_get_double(settings, band_gain[n].data());

    json[bandn]["frequency"] = g_settings_get_double(settings, band_frequency[n].data());

    json[bandn]["q"] = g_settings_get_double(settings, band_q[n].data());
  }
}

void EqualizerPreset::load(const nlohmann::json& json) {
  update_key<bool>(json.at(section).at("equalizer"), settings, "bypass", "bypass");

  update_key<double>(json.at(section).at("equalizer"), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at("equalizer"), settings, "output-gain", "output-gain");

  update_key<gchar*>(json.at(section).at("equalizer"), settings, "mode", "mode");

  update_key<int>(json.at(section).at("equalizer"), settings, "num-bands", "num-bands");

  update_key<bool>(json.at(section).at("equalizer"), settings, "split-channels", "split-channels");

  update_key<double>(json.at(section).at("equalizer"), settings, "balance", "balance");

  update_key<double>(json.at(section).at("equalizer"), settings, "pitch-left", "pitch-left");

  update_key<double>(json.at(section).at("equalizer"), settings, "pitch-right", "pitch-right");

  const auto nbands = g_settings_get_int(settings, "num-bands");

  if (section == "input") {
    load_channel(json.at(section).at("equalizer").at("left"), input_settings_left, nbands);
    load_channel(json.at(section).at("equalizer").at("right"), input_settings_right, nbands);
  } else if (section == "output") {
    load_channel(json.at(section).at("equalizer").at("left"), output_settings_left, nbands);
    load_channel(json.at(section).at("equalizer").at("right"), output_settings_right, nbands);
  }
}

void EqualizerPreset::load_channel(const nlohmann::json& json, GSettings* settings, const int& nbands) {
  for (int n = 0; n < nbands; n++) {
    const auto bandn = "band" + util::to_string(n);

    update_key<gchar*>(json.at(bandn), settings, band_type[n].data(), "type");

    update_key<gchar*>(json.at(bandn), settings, band_mode[n].data(), "mode");

    update_key<gchar*>(json.at(bandn), settings, band_slope[n].data(), "slope");

    update_key<bool>(json.at(bandn), settings, band_solo[n].data(), "solo");

    update_key<bool>(json.at(bandn), settings, band_mute[n].data(), "mute");

    update_key<double>(json.at(bandn), settings, band_gain[n].data(), "gain");

    update_key<double>(json.at(bandn), settings, band_frequency[n].data(), "frequency");

    update_key<double>(json.at(bandn), settings, band_q[n].data(), "q");
  }
}
