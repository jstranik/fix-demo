<main class="container m-3 prose">
    <h1>Fix OMS System Demo</h1>
    <p>The system consists of 3 components: </p>
    <ul>
        <li>Fix Server</li>
        <li>Web Server</li>
        <li>Fix Client</li>
    </ul>

    <section>
        <h2>Fix Server</h2>
        Fix server accepts fix connection. Fix server accepts <a href="https://www.onixs.biz/fix-dictionary/4.4/msgtype_d_68.html">New Single Order</a> message. 

        <p>
        The server has a python interpretter embeeded in and calls into method <b>Logic.should_trade</b>.  It responds appropriately based on the logic in python. 

        <p> 
        The server also exposes <a href="https://developer.mozilla.org/en-US/docs/Web/API/WebSockets_API">WebSocket</a> that a fix OMS website uses to get information about current trades and latest prices. 
    </section>

    <section>
        <h2>Fix Web Site</h2>
        This website has a <a class="li" href="/dashboard">dashboard</a> which shows latest prices of and latest executed trades. The informatino is retrieved from the fix-server using web sockets connection.
    </section>

    <section>
        <h2 class="h2">Fix Client</h2>
        Python application that periodically submits orders using fix api. 
        Can be invoked as <code>nix run github:jstranik/fix-demo#fix-client -- --host stranik.org</code>.
    </section>

    <section>
        Source code available at <a href="https://github.com/jstranik/fix-demo">Github repository</a>.
    </section>
</main>
