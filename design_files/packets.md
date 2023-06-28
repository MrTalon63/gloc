## Packet Types:

-   LG - Location GPS
    Example Packet:

        LG,0,52.5,21.5,118.5,12.5,8,24,5,3.9,12.8,0

    Where

        LG - Packet Type
        0 - Locator ID
        52.5 - Latitude
        21.5 - Longitude
        118.5 - Altitude
        12.5 - Speed
        8 - Satellites used to acquire position
        5 - Accuracy in meters
        3.9 - Battery voltage
        12.8 - Supply voltage (Usually above 12V as it is a car battery)
        0 - Network type enum (0 - GSM, 1 - NB-IoT, 2 - LTE-M, 3 - LoRa, 4 - WiFi, 5 - Bluetooth/BLE, 6 - Mesh Network, 7 - Other/Custom)

    <br>

-   LGE - Location GPS encrypted
    Example Packet:

        LGE,< Encrypted data >

    Where

        LGE - Packet Type
        < Encrypted data > - Encrypted data using AES256 ECB, After decryption it is the same as LG packet.

    <br>

-   LL - Location LBS (Or geolocation using GSM towers, WiFi or Bluetooth beacons)
    Example Packet:

        Packet design is not yet finalized, but it will be similar to LG packet with exception of Latitude, Longitude, Speed, GPS satellites used and Altitude fields.
        In place of those fields there will be a list of GSM towers, WiFi or Bluetooth beacons that will be used to calculate the location using Mozilla Location Service API or selfhosted Ichnaea API in combination with OpenCellID.
        Google Geolocation API will not be used as it is too expensive for such a small project.

    <br>

-   LLE - Location LBS encrypted
    Example Packet:

        LLE,< Encrypted data >

    Where

        LLE - Packet Type
        < Encrypted data > - Encrypted data using AES256 ECB, After decryption it is the same as LL packet.
