// This file is part of k24-default-bitstreams, an accompanying application to fpgad
// (https://github.com/canonical/fpgad) Copyright 2025 Canonical Ltd. SPDX-License-Identifier: GPL-3.0-only
// k24-default-bitstreams is free software: you can redistribute it and/or modify it under the terms of the GNU General
// Public License version 3, as published by the Free Software Foundation. k24-default-bitstreams is distributed in the
// hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
// SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details. You
// should have received a copy of the GNU General Public License along with this program.  If not, see
// http://www.gnu.org/licenses/.

#include <filesystem>
#include <glib-2.0/gio/gio.h>
#include <iostream>
#include <string>

static const char *SERVICE_NAME = "com.canonical.fpgad";
static const char *OBJECT_PATH = "/com/canonical/fpgad/control";
static const char *INTERFACE_NAME = "com.canonical.fpgad.control";

std::string call_set_flags(GDBusConnection *connection, const std::string &platform, const std::string &device,
                           uint32_t flags) {
    GError *error = nullptr;

    GVariant *result = g_dbus_connection_call_sync(
            connection, SERVICE_NAME, OBJECT_PATH, INTERFACE_NAME,
            "SetFpgaFlags", // method
            g_variant_new("(ssu)", platform.c_str(), device.c_str(), flags), // input arguments
            G_VARIANT_TYPE("(s)"), // reply type signature (single string)
            G_DBUS_CALL_FLAGS_NONE,
            10000, // timeout msec
            nullptr, // "cancellable"
            &error // where to store errors
    );

    if (!result) {
        const std::string errMsg = error ? error->message : "Unknown D-Bus error";
        if (error)
            g_error_free(error);
        std::cerr << "D-Bus error in SetFpgaFlags: " << errMsg << std::endl;
        exit(1);
    }

    const char *replyStr;
    g_variant_get(result, "(&s)", &replyStr);
    std::string reply(replyStr);
    g_variant_unref(result);

    return reply;
}

std::string call_write_bitstream(GDBusConnection *connection, const std::string &platform, const std::string &device,
                                 const std::string &bitstream_path, const std::string &fw_lookup_path) {
    GError *error = nullptr;

    GVariant *result =
            g_dbus_connection_call_sync(connection, SERVICE_NAME, OBJECT_PATH, INTERFACE_NAME,
                                        "WriteBitstreamDirect", // method
                                        g_variant_new("(ssss)", platform.c_str(), device.c_str(), // input args
                                                      bitstream_path.c_str(), fw_lookup_path.c_str()),
                                        G_VARIANT_TYPE("(s)"), // reply type signature (single string)
                                        G_DBUS_CALL_FLAGS_NONE,
                                        -1, // timeout msec
                                        nullptr, // "cancellable"
                                        &error // where to store errors
            );

    if (!result) {
        const std::string errMsg = error ? error->message : "Unknown D-Bus error";
        if (error)
            g_error_free(error);
        std::cerr << "D-Bus error in SetFpgaFlags: " << errMsg << std::endl;
        exit(1);
    }

    const char *replyStr;
    g_variant_get(result, "(&s)", &replyStr);
    std::string reply(replyStr);
    g_variant_unref(result);

    return reply;
}

int main(int argc, char *argv[]) {
    // Open a connection to the system bus without making any bindings
    GError *error = nullptr;
    GDBusConnection *conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, &error);
    if (!conn) {
        std::cerr << "Failed to connect to system bus: " << (error ? error->message : "unknown error") << "\n";
        if (error)
            g_error_free(error);
        return 1;
    }
    {
        std::cout << "Calling SetFpgaFlags...\n";
        const std::string reply = call_set_flags(conn, "xlnx", "fpga0", 0u);
        std::cout << "SetFpgaFlags reply: " << reply << "\n";
    }
    {
        std::cout << "Calling WriteBitstreamDirect...\n";
        const std::filesystem::path snapPath(std::getenv("SNAP"));
        const std::string bitstream = (snapPath / "data/k24-starter-kits/k24_starter_kits.bit").string();
        const std::string reply = call_write_bitstream(conn, "xlnx", "fpga0", bitstream, "");
        std::cout << "WriteBitstreamDirect reply: " << reply << "\n";
    }
    g_object_unref(conn);
    return 0;
}
