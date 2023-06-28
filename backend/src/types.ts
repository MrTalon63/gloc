export enum PacketTypes {
	GpsLocation,
	LbsLocation,
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
