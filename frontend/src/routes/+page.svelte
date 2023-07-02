<script>
    import { onMount, onDestroy } from 'svelte';
    import { browser } from '$app/environment';

	function timeNow() {
		var now = new Date();
		now.setMinutes(now.getMinutes() - now.getTimezoneOffset());
		return now.toISOString().slice(0, 16);
	}

	/**
	 * @param {number} hours
	 */
	function timeNowMinusHours(hours) {
		var now = new Date();
		now.setMinutes(now.getMinutes() - now.getTimezoneOffset());
		now.setHours(now.getHours() - hours);
		return now.toISOString().slice(0, 16);
	}

    /**
	 * @type {string | HTMLElement}
	 */
    let mapElement;
	/**
	 * @type {import("leaflet").Map | import("leaflet").LayerGroup<any>}
	 */
	let map;

    onMount(async () => {
        if(browser) {
            const leaflet = await import("leaflet");

            map = leaflet.map(mapElement).setView([52.5, 21.2], 13);

            leaflet.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
                attribution: '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors'
            }).addTo(map);

            leaflet.marker([52.5, 21.5]).addTo(map)
                .bindPopup('A pretty CSS3 popup.<br> Easily customizable.')
                .openPopup();

			leaflet.circle([52.5, 21.4], {
				color: 'red',
				fillColor: '#f03',
				fillOpacity: 0.5,
				radius: 10
			})
			.bindPopup("I am a circle.")
			.addTo(map);
			leaflet.polyline([[52.5, 21.5], [52.5, 21.2]])
				.setStyle({color: 'red'})
				.setLatLngs([[52.5, 21.5], [52.5, 20]])
				.addTo(map);
        }
    });

    onDestroy(async () => {
        if(map) {
            console.log('Unloading Leaflet map.');
            map.remove();
        }
    });
</script>


<main>
	<div class="map" bind:this={mapElement}></div>
	<nav class="bg-primary-subtle">

		<div>
			<label for="vehicle">Pojazd:</label>
			<select id="vehicle">
				<option value="car">Suzuki</option>
				<option value="bike">Toyota</option>
				<option value="foot">BMW</option>
			</select>
		</div>
		<div>
			<label for="since_date">Od:</label>
			<input id="since_date" type="datetime-local" value="{timeNowMinusHours(24)}" />	
		</div>
		<div>
			<label for="till_date">Do:</label>
			<input id="till_date" type="datetime-local" value="{timeNow()}" />
		</div>
		<div>
			<p>Informacje o pojeździe:</p>
			<p>Przebieg: 123456 km</p>
			<p>Spalanie: 6.5 l/100km</p>
			<p>Średnia prędkość: 50 km/h</p>
			<p>Stan paliwa: 30l</p>
			<p>Ostatni ruch: 12:20 19/11/2029</p>
		</div>
	</nav>
</main>

<style>
    @import 'leaflet/dist/leaflet.css';

	nav {
		min-height: 10vh;
	}

    .map {
        height: 90vh;
    }
</style>