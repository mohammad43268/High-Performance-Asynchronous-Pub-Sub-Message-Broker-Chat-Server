import { WebSocketServer } from 'ws';
import net from 'net';
import fs from 'fs';

const WS_PORT = 8081;
const TCP_PORT = 8080;
const TCP_HOST = '127.0.0.1';

const wss = new WebSocketServer({ port: WS_PORT });

console.log('Proxy restarted with logging.');

wss.on('connection', (ws) => {
    const tcpClient = new net.Socket();
    tcpClient.connect(TCP_PORT, TCP_HOST, () => {
        console.log('Connected to TCP');
    });

    tcpClient.on('data', (data) => {
        console.log('TCP->WS:', data.toString());
        const messages = data.toString().split('\n').filter(m => m.trim() !== '');
        for (const msg of messages) {
            ws.send(msg);
        }
    });

    ws.on('message', (message) => {
        console.log('WS->TCP:', message.toString());
        tcpClient.write(message.toString() + '\n');
    });
});
