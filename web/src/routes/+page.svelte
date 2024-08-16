<script lang="ts">
  import { onDestroy, onMount } from "svelte";
  import Layout from "./+layout.svelte";
  import { page } from "$app/stores";
  import { enhance } from "$app/forms";
  import { data } from "autoprefixer";
  let current_symbols: string[] = [];
  let prices = {};
  let trades: { price: number; qty: number; symbol: string; ts: Date }[] = [];

  let socket: WebSocket;

  function updateState(update: any) {
    let { price, trade } = update;
    if (price) {
      let new_prices: any = { ...prices };
      new_prices[price.symbol] = price.price;
      prices = new_prices;
    }
    if (trade) {
      trade.ts = new Date();
      trades = [trade, ...trades].slice(0, 300);
    }
  }
  function openSocket() {
    socket = new WebSocket(`ws://${location.hostname}:9998`);
    socket.onopen = () => {
      console.log("opened");
      //socket.send(JSON.stringify({ cmd: "hello", xx: "text" }));
    };
    socket.onmessage = (e) => {
      const d = JSON.parse(e.data);
      updateState(d);
    };
    socket.onclose = () => {
      console.log("closed");
    };
    socket.onerror = (e) => {
      console.log({ error: e });
    };
  }
  onMount(openSocket);
  function closeSocket() {
    console.log("closing");
    if (socket) socket.close(1000, "Closing due to navigation");
  }
  onDestroy(closeSocket);

  function doreopen() {
    closeSocket();
    openSocket();
  }

  let symbols_to_show: string[] = [];
  $: {
    symbols_to_show = Object.keys(prices).sort();
  }

  function formatDate(date: Date) {
    const hours = date.getHours().toString().padStart(2, "0");
    const minutes = date.getMinutes().toString().padStart(2, "0");
    const seconds = date.getSeconds().toString().padStart(2, "0");
    const milliseconds = date.getMilliseconds().toString().padStart(4, "0");

    return `${hours}:${minutes}:${seconds}.${milliseconds}`;
  }

  let ta
  console.log($page.data)
</script>

<div class="container mx-auto p-4 flex flex-col h-svh">
  <header class="h1 mb-5">Latest Fix Prices</header>
  <section class="">
    This application displays prices submitted by the fix client.
    <p>
      Client connects to the fix-server. Fix server receives an order, invokes a
      python logic to determine wheter to trade.
    </p>
    <p>
      The server also exposes a web socket that shows current prices of symbols.
    </p>
  </section>
  <main class="flex-grow gap-2 flex flex-col md:flex-row overflow-hidden">
    <table class="table table-fixed basis-1/2">
      <caption class="text-lg">Latest prices</caption>
      <thead>
        <tr><th>Symbol</th><th>Price</th></tr>
      </thead>
      <tbody>
        {#each symbols_to_show as symbol}
          <tr>
            <td>{symbol}</td>
            <td>$ {prices[symbol].toFixed(2)}</td>
          </tr>
        {/each}
      </tbody>
    </table>
    <div class="basis-1/2 table-container overflow-y-visible">
      <table class="table table-fixed">
        <caption class="text-lg">Latest executions</caption>
        <thead>
          <tr><th>Time</th><th>Symbol</th><th>Price</th><th>Qty</th></tr>
        </thead>
        <tbody class="">
          {#each trades as trade (trade.ts)}
            <tr>
              <td>{formatDate(trade.ts)}</td>
              <td>{trade.symbol}</td>
              <td>$ {trade.price.toFixed(2)}</td>
              <td>{trade.qty}</td>
            </tr>
          {/each}
        </tbody>
      </table>
    </div>
  </main>
  <section>
    <form method="post" use:enhance>
        <label>
        Logic Script
        <textarea name="logic" bind:this={ta} class="textarea" rows="4" placeholder="Logic Script" >{$page.form.data.logic || $page.data.logic}</textarea>
        </label>
        <input type="submit" class="btn variant-ghost-primary"/>
    </form>
</div>
