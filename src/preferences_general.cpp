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

#include "preferences_general.hpp"

namespace ui::preferences::general {

using namespace std::string_literals;

auto constexpr log_tag = "preferences_general: ";

struct _PreferencesGeneral {
  AdwPreferencesPage parent_instance;

  GtkSwitch *enable_autostart, *process_all_inputs, *process_all_outputs, *theme_switch, *shutdown_on_window_close,
      *use_cubic_volumes, *autohide_popovers, *reset_volume_on_startup;

  GtkSpinButton* inactivity_timeout;

  GSettings* settings;
};

G_DEFINE_TYPE(PreferencesGeneral, preferences_general, ADW_TYPE_PREFERENCES_PAGE)

auto on_enable_autostart(GtkSwitch* obj, gboolean state, gpointer user_data) -> gboolean {
  std::filesystem::path autostart_dir{g_get_user_config_dir() + "/autostart"s};

  if (!std::filesystem::is_directory(autostart_dir)) {
    std::filesystem::create_directories(autostart_dir);
  }

  std::filesystem::path autostart_file{g_get_user_config_dir() + "/autostart/easyeffects-service.desktop"s};

  if (state != 0) {
    if (!std::filesystem::exists(autostart_file)) {
      std::ofstream ofs{autostart_file};

      ofs << "[Desktop Entry]\n";
      ofs << "Name=EasyEffects\n";
      ofs << "Comment=EasyEffects Service\n";
      ofs << "Exec=easyeffects --gapplication-service\n";
      ofs << "Icon=easyeffects\n";
      ofs << "StartupNotify=false\n";
      ofs << "Terminal=false\n";
      ofs << "Type=Application\n";

      ofs.close();

      util::debug(log_tag + "autostart file created"s);
    }
  } else {
    if (std::filesystem::exists(autostart_file)) {
      std::filesystem::remove(autostart_file);

      util::debug(log_tag + "autostart file removed"s);
    }
  }

  return 0;
}

void dispose(GObject* object) {
  auto* self = EE_PREFERENCES_GENERAL(object);

  g_object_unref(self->settings);

  util::debug(log_tag + "disposed"s);

  G_OBJECT_CLASS(preferences_general_parent_class)->dispose(object);
}

void preferences_general_class_init(PreferencesGeneralClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;

  gtk_widget_class_set_template_from_resource(widget_class, "/com/github/wwmm/easyeffects/ui/preferences_general.ui");

  gtk_widget_class_bind_template_child(widget_class, PreferencesGeneral, enable_autostart);
  gtk_widget_class_bind_template_child(widget_class, PreferencesGeneral, process_all_inputs);
  gtk_widget_class_bind_template_child(widget_class, PreferencesGeneral, process_all_outputs);
  gtk_widget_class_bind_template_child(widget_class, PreferencesGeneral, theme_switch);
  gtk_widget_class_bind_template_child(widget_class, PreferencesGeneral, autohide_popovers);
  gtk_widget_class_bind_template_child(widget_class, PreferencesGeneral, shutdown_on_window_close);
  gtk_widget_class_bind_template_child(widget_class, PreferencesGeneral, use_cubic_volumes);
  gtk_widget_class_bind_template_child(widget_class, PreferencesGeneral, reset_volume_on_startup);
  gtk_widget_class_bind_template_child(widget_class, PreferencesGeneral, inactivity_timeout);

  gtk_widget_class_bind_template_callback(widget_class, on_enable_autostart);
}

void preferences_general_init(PreferencesGeneral* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->settings = g_settings_new("com.github.wwmm.easyeffects");

  prepare_spinbuttons<"s">(self->inactivity_timeout);

  // initializing some widgets

  gtk_switch_set_active(self->enable_autostart,
                        static_cast<gboolean>(std::filesystem::is_regular_file(
                            g_get_user_config_dir() + "/autostart/easyeffects-service.desktop"s)));

  gsettings_bind_widgets<"process-all-inputs", "process-all-outputs", "use-dark-theme", "shutdown-on-window-close",
                         "use-cubic-volumes", "autohide-popovers", "reset-volume-on-startup", "inactivity-timeout">(
      self->settings, self->process_all_inputs, self->process_all_outputs, self->theme_switch,
      self->shutdown_on_window_close, self->use_cubic_volumes, self->autohide_popovers, self->reset_volume_on_startup,
      self->inactivity_timeout);
}

auto create() -> PreferencesGeneral* {
  return static_cast<PreferencesGeneral*>(g_object_new(EE_TYPE_PREFERENCES_GENERAL, nullptr));
}

}  // namespace ui::preferences::general
