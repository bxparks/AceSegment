#!/usr/bin/gawk -f
#
# Usage: generate_table.awk < ${board}.txt
#
# Takes the file generated by collect.sh and generates an ASCII
# table that can be inserted into the README.md.

BEGIN {
  labels[0] = "baseline"
  labels[1] = "Scanning(Direct)";
  labels[2] = "Scanning(DirectFast)";
  labels[3] = "Scanning(Single,SwSpi)";
  labels[4] = "Scanning(Single,SwSpiFast)";
  labels[5] = "Scanning(Single,HwSpi)";
  labels[6] = "Scanning(Single,HwSpiFast)";
  labels[7] = "Scanning(Dual,SwSpi)";
  labels[8] = "Scanning(Dual,SwSpiFast)";
  labels[9] = "Scanning(Dual,HwSpi)";
  labels[10] = "Scanning(Dual,HwSpiFast)";
  labels[11] = "Tm1637(Wire)";
  labels[12] = "Tm1637(WireFast)";
  labels[13] = "Max7219(SwSpi)";
  labels[14] = "Max7219(SwSpiFast)";
  labels[15] = "Max7219(HwSpi)";
  labels[16] = "Max7219(HwSpiFast)";
  labels[17] = "StubModule+LedDisplay";
  labels[18] = "NumberWriter+Stub";
  labels[19] = "ClockWriter+Stub";
  labels[20] = "TemperatureWriter+Stub";
  labels[21] = "CharWriter+Stub";
  labels[22] = "StringWriter+Stub";
  record_index = 0
}
{
  u[record_index]["flash"] = $2
  u[record_index]["ram"] = $4
  record_index++
}
END {
  NUM_ENTRIES = record_index

  # Calculate the flash and memory deltas from baseline
  base_flash = u[0]["flash"]
  base_ram = u[0]["ram"]
  for (i = 0; i < NUM_ENTRIES; i++) {
    if (u[i]["flash"] != "-1") {
      u[i]["d_flash"] = u[i]["flash"] - base_flash
      u[i]["d_ram"] = u[i]["ram"] - base_ram
    } else {
      u[i]["d_flash"] = -1
      u[i]["d_ram"] = -1
    }
  }

  printf("+--------------------------------------------------------------+\n")
  printf("| functionality                   |  flash/  ram |       delta |\n")
  printf("|---------------------------------+--------------+-------------|\n")
  printf("| %-31s | %6d/%5d | %5d/%5d |\n",
    labels[0], u[0]["flash"], u[0]["ram"], u[0]["d_flash"], u[0]["d_ram"])
  for (i = 1 ; i < NUM_ENTRIES; i++) {
    if (labels[i] ~ /Scanning\(Direct\)/ \
        || labels[i] ~ /Scanning\(Single,SwSpi\)/ \
        || labels[i] ~ /Scanning\(Dual,SwSpi\)/ \
        || labels[i] ~ /Tm1637\(Wire\)/ \
        || labels[i] ~ /Max7219\(SwSpi\)/ \
        || labels[i] == "StubModule+LedDisplay") {
      printf(\
        "|---------------------------------+--------------+-------------|\n")
    }
    printf("| %-31s | %6d/%5d | %5d/%5d |\n",
        labels[i], u[i]["flash"], u[i]["ram"], u[i]["d_flash"], u[i]["d_ram"])
  }
  printf("+--------------------------------------------------------------+\n")
}
