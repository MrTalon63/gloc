import net from "node:net";

import db from "./mysql";
import config from "./config";
import { NetworkType } from "./types";

async function tcp() {
	const server = net.createServer();

	server.on("connection", connectionHandler);

	server.listen(config.tcpServer.port, config.tcpServer.host, () => {
		console.log(`TCP server listening on ${config.tcpServer.host}:${config.tcpServer.port}`);
	});
}

async function connectionHandler(socket: net.Socket) {
	console.log(`New connection from ${socket.remoteAddress}:${socket.remotePort}`);

	socket.on("data", (data) => {
		console.log(`Received data from ${socket.remoteAddress}:${socket.remotePort}: ${data.toString("utf8")}`);
		packetParser(data.toString("utf8").trim());
	});

	socket.on("close", () => {
		console.log(`Connection from ${socket.remoteAddress}:${socket.remotePort} closed`);
	});
}

async function packetParser(packet: string): Promise<void> {
	const data = packet.split(",");

	switch (data[0]) {
		case "LG":
			LGHandler(data);
			return;
		case "LGE":
			// Packet support is not implemented yet
			return;
		case "LL":
			// Packet support is not implemented yet
			return;
		case "LLE":
			// Packet support is not implemented yet
			return;
		default:
			throw new Error("Invalid packet type");
	}
}

async function LGHandler(packet: string[]) {
	packet.shift(); // Remove packet type

	const packetData = {
		date: new Date(),
		locatorId: parseInt(packet.shift() as string),
		lat: parseFloat(packet.shift() as string),
		lng: parseFloat(packet.shift() as string),
		altitude: parseFloat(packet.shift() as string),
		speed: parseFloat(packet.shift() as string),
		satellites: parseInt(packet.shift() as string),
		accuracy: parseFloat(packet.shift() as string),
		vbat: parseFloat(packet.shift() as string),
		vcc: parseFloat(packet.shift() as string),
		signal: parseInt(packet.shift() as string),
		networkType: parseInt(packet.shift() as string) as NetworkType,
		networkOperator: packet.shift() as string,
	};

	if (packetData.signal === 0) packetData.signal = -115;
	if (packetData.signal === 1) packetData.signal = -111;
	if (packetData.signal >= 2 && packetData.signal <= 30) packetData.signal = numberMap(packetData.signal, 2, 30, -110, -54);
	if (packetData.signal === 31) packetData.signal = -52;
	if (packetData.signal === 99) packetData.signal = 250;

	console.log("Received LG packet:");
	console.log(packetData);

	if (packetData.lat === 0 && packetData.lng === 0) return;

	const googleLocation = "https://www.google.com/maps/place/" + packetData.lat + "," + packetData.lng;
	db.execute("INSERT INTO `locator_" + packetData.locatorId + "` (`ts`, `lat`, `lng`, `alt`, `speed`, `sats`, `accuracy`, `googleLocation`, `vbat`, `vcc`, `sigstr`, `networkType`, `networkOperator`) VALUES (?, ?, ?, ?, ?, ?, ? ,? ,? ,? ,? ,? ,?)", [packetData.date, packetData.lat, packetData.lng, packetData.altitude, packetData.speed, packetData.satellites, packetData.accuracy, googleLocation, packetData.vbat, packetData.vcc, packetData.signal, packetData.networkType, packetData.networkOperator]);
}

function numberMap(num: number, in_min: number, in_max: number, out_min: number, out_max: number) {
	return ((num - in_min) * (out_max - out_min)) / (in_max - in_min) + out_min;
}

export default tcp;
