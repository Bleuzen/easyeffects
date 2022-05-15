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

#include "filter_ui.hpp"

namespace ui::filter_box {

using namespace std::string_literals;

auto constexpr log_tag = "filter_box: ";

struct Data {
 public:
  ~Data() { util::debug(log_tag + "data struct destroyed"s); }

  std::shared_ptr<Filter> filter;

  std::vector<sigc::connection> connections;

  std::vector<gulong> gconnections;
};

struct _FilterBox {
  GtkBox parent_instance;

  GtkScale *input_gain, *output_gain;

  GtkLevelBar *input_level_left, *input_level_right, *output_level_left, *output_level_right;

  GtkLabel *input_level_left_label, *input_level_right_label, *output_level_left_label, *output_level_right_label;

  GtkToggleButton* bypass;

  GtkComboBoxText* mode;

  GtkSpinButton *frequency, *resonance, *inertia;

  GSettings* settings;

  Data* data;
};

G_DEFINE_TYPE(FilterBox, filter_box, GTK_TYPE_BOX)

void on_bypass(FilterBox* self, GtkToggleButton* btn) {
  self->data->filter->bypass = gtk_toggle_button_get_active(btn);
}

void on_reset(FilterBox* self, GtkButton* btn) {
  gtk_toggle_button_set_active(self->bypass, 0);

  util::reset_all_keys(self->settings);
}

void setup(FilterBox* self, std::shared_ptr<Filter> filter, const std::string& schema_path) {
  self->data->filter = filter;

  self->settings = g_settings_new_with_path("com.github.wwmm.easyeffects.filter", schema_path.c_str());

  filter->post_messages = true;
  filter->bypass = false;

  self->data->connections.push_back(filter->input_level.connect([=](const float& left, const float& right) {
    update_level(self->input_level_left, self->input_level_left_label, self->input_level_right,
                 self->input_level_right_label, left, right);
  }));

  self->data->connections.push_back(filter->output_level.connect([=](const float& left, const float& right) {
    update_level(self->output_level_left, self->output_level_left_label, self->output_level_right,
                 self->output_level_right_label, left, right);
  }));

  gsettings_bind_widgets<"input-gain", "output-gain">(self->settings, self->input_gain, self->output_gain);

  g_settings_bind(self->settings, "frequency", gtk_spin_button_get_adjustment(self->frequency), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "resonance", gtk_spin_button_get_adjustment(self->resonance), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "inertia", gtk_spin_button_get_adjustment(self->inertia), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "mode", self->mode, "active-id", G_SETTINGS_BIND_DEFAULT);
}

void dispose(GObject* object) {
  auto* self = EE_FILTER_BOX(object);

  self->data->filter->bypass = false;

  for (auto& c : self->data->connections) {
    c.disconnect();
  }

  for (auto& handler_id : self->data->gconnections) {
    g_signal_handler_disconnect(self->settings, handler_id);
  }

  self->data->connections.clear();
  self->data->gconnections.clear();

  g_object_unref(self->settings);

  util::debug(log_tag + "disposed"s);

  G_OBJECT_CLASS(filter_box_parent_class)->dispose(object);
}

void finalize(GObject* object) {
  auto* self = EE_FILTER_BOX(object);

  delete self->data;

  util::debug(log_tag + "finalized"s);

  G_OBJECT_CLASS(filter_box_parent_class)->finalize(object);
}

void filter_box_class_init(FilterBoxClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;
  object_class->finalize = finalize;

  gtk_widget_class_set_template_from_resource(widget_class, "/com/github/wwmm/easyeffects/ui/filter.ui");

  gtk_widget_class_bind_template_child(widget_class, FilterBox, input_gain);
  gtk_widget_class_bind_template_child(widget_class, FilterBox, output_gain);
  gtk_widget_class_bind_template_child(widget_class, FilterBox, input_level_left);
  gtk_widget_class_bind_template_child(widget_class, FilterBox, input_level_right);
  gtk_widget_class_bind_template_child(widget_class, FilterBox, output_level_left);
  gtk_widget_class_bind_template_child(widget_class, FilterBox, output_level_right);
  gtk_widget_class_bind_template_child(widget_class, FilterBox, input_level_left_label);
  gtk_widget_class_bind_template_child(widget_class, FilterBox, input_level_right_label);
  gtk_widget_class_bind_template_child(widget_class, FilterBox, output_level_left_label);
  gtk_widget_class_bind_template_child(widget_class, FilterBox, output_level_right_label);

  gtk_widget_class_bind_template_child(widget_class, FilterBox, bypass);

  gtk_widget_class_bind_template_child(widget_class, FilterBox, mode);
  gtk_widget_class_bind_template_child(widget_class, FilterBox, frequency);
  gtk_widget_class_bind_template_child(widget_class, FilterBox, resonance);
  gtk_widget_class_bind_template_child(widget_class, FilterBox, inertia);

  gtk_widget_class_bind_template_callback(widget_class, on_bypass);
  gtk_widget_class_bind_template_callback(widget_class, on_reset);
}

void filter_box_init(FilterBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->data = new Data();

  prepare_scales<"dB">(self->input_gain, self->output_gain);

  prepare_spinbuttons<"Hz">(self->frequency);
  prepare_spinbuttons<"dB">(self->resonance);
  prepare_spinbuttons<"ms">(self->inertia);
}

auto create() -> FilterBox* {
  return static_cast<FilterBox*>(g_object_new(EE_TYPE_FILTER_BOX, nullptr));
}

}  // namespace ui::filter_box
