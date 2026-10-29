#pragma once

// Wi-Fi for the controller: joins the saved network, reconnects after drops
// and, when there is no network to join, opens an open setup network whose
// page lists nearby networks, tests the password and shows the result.
//
// The setup network opens:
// - at once when no network is saved;
// - when a network is saved but the board has not connected to it within
//   10 minutes of starting, closing again as soon as it connects;
// - on request (POST /setup/open), while the board stays on its current
//   network until a new one works. It closes after 10
//   minutes if no new network is set up.
// It never opens because of a drop after the board has connected.

#include <WebServer.h>

// Call once from setup(), before server.begin(). Registers the setup
// routes on `server` (all but "/") and starts joining the saved network.
void wifiSetupBegin(WebServer& server, const char* hostname, const char* setupNetwork);

// Call on every loop iteration.
void wifiSetupLoop();

// For "/": sends the setup page and returns true when the request came in on
// the setup network; returns false otherwise.
bool wifiSetupHandleRoot();

// Seconds until a setup network opened on request closes, or -1 when it is
// not open on request.
long wifiSetupSecondsLeft();

// Forgets the saved network. The caller restarts the board afterwards.
void wifiForgetNetwork();
