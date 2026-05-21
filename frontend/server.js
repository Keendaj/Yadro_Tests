import express from 'express';
import { WebSocketServer } from 'ws';
import net from 'net';
import path from 'path';
import { fileURLToPath } from 'url';

const __dirname = path.dirname(fileURLToPath(import.meta.url));

const app = express();
const PORT = 8080;
const SOCKET_PATH = '/tmp/system_data.sock';

app.use(express.static(path.join(__dirname, 'dist')));

const server = app.listen(PORT, () => {
    console.log(`[Bridge] Web UI running on http://localhost:${PORT}`);
});

const wss = new WebSocketServer({ server });

const udsClient = net.createConnection(SOCKET_PATH);

udsClient.on('connect', () => {
    console.log('[Bridge] Connected to C++ UDS Backend');
});

let buffer = '';
udsClient.on('data', (data) => {
    buffer += data.toString();
    let lines = buffer.split('\n');
    buffer = lines.pop();

    lines.forEach(line => {
        if (line.trim()) {
            wss.clients.forEach(client => {
                if (client.readyState === 1) {
                    client.send(line);
                }
            });
        }
    });
});

udsClient.on('error', (err) => {
    console.error('[Bridge] UDS Error:', err.message);
});