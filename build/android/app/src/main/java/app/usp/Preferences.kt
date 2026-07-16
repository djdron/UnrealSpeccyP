/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2026 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

package app.usp

import android.app.UiModeManager
import android.content.SharedPreferences
import android.content.SharedPreferences.OnSharedPreferenceChangeListener
import android.os.Build
import android.os.Bundle
import android.view.MenuItem
import android.view.View
import android.view.Window
import androidx.core.view.ViewCompat
import androidx.core.view.WindowInsetsCompat
import androidx.preference.Preference
import androidx.preference.PreferenceGroup
import androidx.preference.SwitchPreferenceCompat
import androidx.preference.ListPreference
import androidx.fragment.app.FragmentActivity
import androidx.preference.PreferenceFragmentCompat
import androidx.recyclerview.widget.LinearLayoutManager

open class Preferences : FragmentActivity() {

	override fun onCreate(savedInstanceState: Bundle?) {
		window.requestFeature(Window.FEATURE_OPTIONS_PANEL)
		super.onCreate(savedInstanceState)
		supportFragmentManager.beginTransaction()
			.replace(android.R.id.content, PreferencesFragment())
			.commit()
	}

	override fun onOptionsItemSelected(item: MenuItem): Boolean {
		if (item.itemId == android.R.id.home) {
			onBackPressedDispatcher.onBackPressed()
		}
		return super.onOptionsItemSelected(item)
	}

	companion object {
		const val use_sensor_id = "use sensor"
		const val use_keyboard_id = "use keyboard"
	}

	class PreferencesFragment : PreferenceFragmentCompat(), OnSharedPreferenceChangeListener {

		private lateinit var select_joystick: ListPreference
		private lateinit var use_sensor: SwitchPreferenceCompat
		private lateinit var sound_chip: ListPreference
		private lateinit var sound_chip_stereo: ListPreference
		private lateinit var save_slot: ListPreference
		private lateinit var save_file: Preference
		private lateinit var select_drive: ListPreference
		private lateinit var tape: Preference
		private lateinit var auto_play_image: SwitchPreferenceCompat
		private lateinit var tape_fast: SwitchPreferenceCompat
		private lateinit var mode_48k: SwitchPreferenceCompat
		private lateinit var reset_to_service_rom: SwitchPreferenceCompat
		private lateinit var select_zoom: ListPreference
		private lateinit var filtering: SwitchPreferenceCompat
		private lateinit var gigascreen: SwitchPreferenceCompat
		private lateinit var black_and_white: SwitchPreferenceCompat
		private lateinit var theme: ListPreference

		companion object {
			private const val select_joystick_id = "joystick"
			private const val sound_chip_id = "sound chip"
			private const val sound_chip_stereo_id = "ay stereo"
			private const val auto_play_image_id = "auto play image"
			private const val save_slot_id = "save slot"
			private const val save_file_id = "save file"
			private const val select_drive_id = "drive"
			private const val tape_id = "tape"
			private const val tape_fast_id = "fast tape"
			private const val mode_48k_id = "mode 48k"
			private const val reset_to_service_rom_id = "reset to service rom"
			private const val select_zoom_id = "zoom"
			private const val filtering_id = "filtering"
			private const val gigascreen_id = "gigascreen"
			private const val black_and_white_id = "black and white"
			private const val theme_id = "theme"

			private var scroll_stored = 0
			private var scroll_offset_stored = 0
		}

		override fun onCreatePreferences(savedInstanceState: Bundle?, rootKey: String?) {
			setPreferencesFromResource(R.xml.preferences, rootKey)
			removeIconSpace(preferenceScreen)
			select_joystick = findPreference(select_joystick_id)!!
			use_sensor = findPreference(use_sensor_id)!!
			sound_chip = findPreference(sound_chip_id)!!
			sound_chip_stereo = findPreference(sound_chip_stereo_id)!!
			auto_play_image = findPreference(auto_play_image_id)!!
			save_slot = findPreference(save_slot_id)!!
			save_file = findPreference(save_file_id)!!
			select_drive = findPreference(select_drive_id)!!
			tape = findPreference(tape_id)!!
			tape_fast = findPreference(tape_fast_id)!!
			mode_48k = findPreference(mode_48k_id)!!
			reset_to_service_rom = findPreference(reset_to_service_rom_id)!!
			select_zoom = findPreference(select_zoom_id)!!
			filtering = findPreference(filtering_id)!!
			gigascreen = findPreference(gigascreen_id)!!
			black_and_white = findPreference(black_and_white_id)!!
			theme = findPreference(theme_id)!!
			LoadValues()
			UpdateDescs()

			tape.setOnPreferenceClickListener {
				Emulator.the.TapeToggle()
				UpdateDescs()
				true
			}

			save_file.setOnPreferenceClickListener {
				Emulator.the.SaveFile()
				UpdateDescs()
				true
			}
		}
		private fun removeIconSpace(preference: Preference) {
			preference.isIconSpaceReserved = false

			if (preference is PreferenceGroup) {
				for (i in 0 until preference.preferenceCount) {
					removeIconSpace(preference.getPreference(i))
				}
			}
		}
		override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
			super.onViewCreated(view, savedInstanceState)

			ViewCompat.setOnApplyWindowInsetsListener(view) { v, insets ->
				val systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars())
				v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom)
				insets
			}

			if (scroll_stored != 0 || scroll_offset_stored != 0) {
				val lm = listView.layoutManager as LinearLayoutManager
				lm.scrollToPositionWithOffset(scroll_stored, scroll_offset_stored)
			}
		}
		override fun onResume() {
			super.onResume()
			preferenceScreen.sharedPreferences?.registerOnSharedPreferenceChangeListener(this)
		}
		override fun onPause() {
			super.onPause()
			Emulator.the.StoreOptions()

			val lm = listView.layoutManager as LinearLayoutManager
			scroll_stored = lm.findFirstVisibleItemPosition()

			val topView = lm.getChildAt(0)
			scroll_offset_stored = if (topView == null) 0 else topView.top - listView.paddingTop

			preferenceScreen.sharedPreferences?.unregisterOnSharedPreferenceChangeListener(this)
		}

		private fun LoadValues() {
			select_joystick.setValueIndex(Emulator.the.GetOptionInt(select_joystick_id))
			use_sensor.isChecked = Emulator.the.GetOptionBool(use_sensor_id)
			sound_chip.setValueIndex(Emulator.the.GetOptionInt(sound_chip_id))
			sound_chip_stereo.setValueIndex(Emulator.the.GetOptionInt(sound_chip_stereo_id))
			auto_play_image.isChecked = Emulator.the.GetOptionBool(auto_play_image_id)
			save_slot.setValueIndex(Emulator.the.GetOptionInt(save_slot_id))
			select_drive.setValueIndex(Emulator.the.GetOptionInt(select_drive_id))
			tape_fast.isChecked = Emulator.the.GetOptionBool(tape_fast_id)
			mode_48k.isChecked = Emulator.the.GetOptionBool(mode_48k_id)
			reset_to_service_rom.isChecked = Emulator.the.GetOptionBool(reset_to_service_rom_id)
			select_zoom.setValueIndex(Emulator.the.GetOptionInt(select_zoom_id))
			filtering.isChecked = Emulator.the.GetOptionBool(filtering_id)
			gigascreen.isChecked = Emulator.the.GetOptionBool(gigascreen_id)
			black_and_white.isChecked = Emulator.the.GetOptionBool(black_and_white_id)
		}

		private fun UpdateDescs() {
			when (Emulator.the.TapeState()) {
				0 -> {
					tape.setSummary(R.string.tape_na)
					tape.isEnabled = false
				}
				1 -> {
					tape.setSummary(R.string.tape_stopped)
					tape.isEnabled = true
				}
				2 -> {
					tape.setSummary(R.string.tape_started)
					tape.isEnabled = true
				}
			}
			save_file.isEnabled = Emulator.the.DiskChanged()
		}

		override fun onSharedPreferenceChanged(sharedPreferences: SharedPreferences, key: String?) {
			when (key) {
				select_joystick_id ->	Emulator.the.SetOptionInt(select_joystick_id, select_joystick.value.toInt())
				sound_chip_id -> 		Emulator.the.SetOptionInt(sound_chip_id, sound_chip.value.toInt())
				sound_chip_stereo_id -> Emulator.the.SetOptionInt(sound_chip_stereo_id, sound_chip_stereo.value.toInt())
				auto_play_image_id -> 	Emulator.the.SetOptionBool(auto_play_image_id, auto_play_image.isChecked)
				save_slot_id -> 		Emulator.the.SetOptionInt(save_slot_id, save_slot.value.toInt())
				select_drive_id -> 		Emulator.the.SetOptionInt(select_drive_id, select_drive.value.toInt())
				mode_48k_id -> 			Emulator.the.SetOptionBool(mode_48k_id, mode_48k.isChecked)
				reset_to_service_rom_id -> Emulator.the.SetOptionBool(reset_to_service_rom_id, reset_to_service_rom.isChecked)
				tape_fast_id -> 		Emulator.the.SetOptionBool(tape_fast_id, tape_fast.isChecked)
				select_zoom_id -> 		Emulator.the.SetOptionInt(select_zoom_id, select_zoom.value.toInt())
				use_sensor_id -> 		Emulator.the.SetOptionBool(use_sensor_id, use_sensor.isChecked)
				filtering_id -> 		Emulator.the.SetOptionBool(filtering_id, filtering.isChecked)
				gigascreen_id -> 		Emulator.the.SetOptionBool(gigascreen_id, gigascreen.isChecked)
				black_and_white_id -> 	Emulator.the.SetOptionBool(black_and_white_id, black_and_white.isChecked)
				theme_id -> {
					val uiModeManager = requireActivity().getSystemService(UI_MODE_SERVICE) as UiModeManager
					val mode = when (theme.value) {
						"dark" -> UiModeManager.MODE_NIGHT_YES
						"light" -> UiModeManager.MODE_NIGHT_NO
						else -> UiModeManager.MODE_NIGHT_AUTO
					}
					if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) { // Android 12+
						uiModeManager.setApplicationNightMode(mode)
					} else {
						uiModeManager.setNightMode(mode)
					}
				}
			}
		}
	}
}
