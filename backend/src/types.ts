export enum NetworkType {
	GSM = 1,
	NB_IoT = 2,
	LTE_M = 3,
	LoRa = 4,
	WiFi = 5,
	Bluetooth = 6,
	Mesh = 7,
	Other = 8,
}

export interface GsmPacket {
	encryption: boolean;
	id: number;
	lat: number;
	lng: number;
	speed: number;
	sats: number;
	signal: number;
	operator?: string;

	batteryVoltage: number;
	supplyVoltage: number;
}
