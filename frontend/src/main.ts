import Chart from 'chart.js/auto';

interface ProcessData {
    id: number;
    user: string;
    priority: number;
    state: string;
    virtual_memory: number;
    resident_memory: number;
    cpu_load: number;
    mem_load: number;
    execution_time: number;
    command: string;
}

interface SystemData {
    ram: {
        total: number;
        available: number;
        used: number;
        cached: number;
        buffer: number;
        swap_total: number;
        swap_used: number;
        swap_free: number;
    };
    cpu_load: number;
    cpu_cores: number[];
    processes: ProcessData[];
    
    load_avg: [number, number, number];
    uptime_sec: number;
    tasks_total: number;
    tasks_running: number;
}

function formatBytes(bytes: number, decimals = 2): string {
    if (!+bytes) return '0 B';
    const k = 1024;
    const dm = decimals < 0 ? 0 : decimals;
    const sizes = ['B', 'KB', 'MB', 'GB', 'TB'];
    const i = Math.floor(Math.log(bytes) / Math.log(k));
    return `${parseFloat((bytes / Math.pow(k, i)).toFixed(dm))} ${sizes[i]}`;
}

function formatTimePlus(seconds: number): string {
    const mins = Math.floor(seconds / 60);
    const secs = Math.floor(seconds % 60);
    const hundredths = Math.floor((seconds % 1) * 100);
    return `${mins}:${secs.toString().padStart(2, '0')}.${hundredths.toString().padStart(2, '0')}`;
}

function formatUptime(totalSeconds: number): string {
    const hours = Math.floor(totalSeconds / 3600);
    const minutes = Math.floor((totalSeconds % 3600) / 60);
    const seconds = totalSeconds % 60;
    return `${hours.toString().padStart(2, '0')}:${minutes.toString().padStart(2, '0')}:${seconds.toString().padStart(2, '0')}`;
}

function getStateColor(state: string): string {
    if (state.includes('R')) return 'bg-green-500/20 text-green-400 border-green-500/30';
    if (state.includes('S')) return 'bg-gray-500/20 text-gray-400 border-gray-500/30';
    if (state.includes('Z')) return 'bg-red-500/20 text-red-400 border-red-500/30';
    return 'bg-blue-500/20 text-blue-400 border-blue-500/30';
}

function getCoreColor(load: number): string {
    if (load < 50) return 'bg-green-500';
    if (load < 85) return 'bg-yellow-500';
    return 'bg-red-500';
}

const commonChartOptions = {
    responsive: true,
    maintainAspectRatio: false,
    plugins: {
        legend: { position: 'right' as const, labels: { color: '#94a3b8', boxWidth: 12 } }
    },
    cutout: '70%',
    borderWidth: 0
};

const cpuCtx = (document.getElementById('cpuChart') as HTMLCanvasElement).getContext('2d')!;
const cpuChart = new Chart(cpuCtx, {
    type: 'doughnut',
    data: {
        labels: ['Used', 'Idle'],
        datasets: [{
            data: [0, 100],
            backgroundColor: ['#10b981', '#022c22']
        }]
    },
    options: {
        ...commonChartOptions,
        plugins: {
            ...commonChartOptions.plugins,
            tooltip: { callbacks: { label: (ctx) => ` ${Number(ctx.raw).toFixed(1)}%` } }
        }
    }
});

const ramCtx = (document.getElementById('ramChart') as HTMLCanvasElement).getContext('2d')!;
const ramChart = new Chart(ramCtx, {
    type: 'doughnut',
    data: {
        labels: ['Used (Apps)', 'Cached', 'Buffers', 'Free'],
        datasets: [{
            data: [0, 0, 0, 100],
            backgroundColor: ['#047857', '#10b981', '#34d399', '#022c22']
        }]
    },
    options: {
        ...commonChartOptions,
        plugins: {
            ...commonChartOptions.plugins,
            tooltip: { callbacks: { label: (ctx) => ` ${formatBytes(Number(ctx.raw))}` } }
        }
    }
});

function updateUI(data: SystemData) {
    document.getElementById('system-stats')!.innerHTML = `
        <div>Tasks: <span class="text-emerald-100">${data.tasks_total}</span>, <span class="text-emerald-400">${data.tasks_running} running</span></div>
        <div>Load average: <span class="text-white">${data.load_avg[0].toFixed(2)}</span> ${data.load_avg[1].toFixed(2)} ${data.load_avg[2].toFixed(2)}</div>
        <div>Uptime: <span class="text-emerald-300">${formatUptime(data.uptime_sec)}</span></div>
    `;

    const cpuUsed = data.cpu_load;
    const cpuIdle = Math.max(0, 100 - cpuUsed);
    cpuChart.data.datasets[0].data = [cpuUsed, cpuIdle];
    cpuChart.update();
    document.getElementById('cpu-text')!.textContent = `${cpuUsed.toFixed(1)}%`;

    const coresContainer = document.getElementById('cpuCores')!;
    coresContainer.innerHTML = data.cpu_cores.map((load, index) => `
        <div class="flex items-center gap-3 text-sm">
            <span class="text-emerald-400 font-mono w-12">CPU${index}</span>
            <div class="flex-1 h-3 bg-emerald-950 rounded-full overflow-hidden border border-emerald-800">
                <div class="h-full ${getCoreColor(load)} transition-all duration-300 ease-out" style="width: ${load}%"></div>
            </div>
            <span class="text-emerald-300 font-mono w-10 text-right">${load.toFixed(0)}%</span>
        </div>
    `).join('');

    const freeRam = Math.max(0, data.ram.total - data.ram.used - data.ram.cached - data.ram.buffer);
    ramChart.data.datasets[0].data = [data.ram.used, data.ram.cached, data.ram.buffer, freeRam];
    ramChart.update();

    document.getElementById('ram-text')!.innerHTML = `
        <div class="text-xs text-emerald-400 mt-2 text-center">
            Total: <span class="text-emerald-100 font-mono">${formatBytes(data.ram.total)}</span><br>
            Swap Used: <span class="text-emerald-100 font-mono">${formatBytes(data.ram.swap_used)}</span> / ${formatBytes(data.ram.swap_total)}
        </div>
    `;

    const tbody = document.getElementById('processTableBody')!;
    const topProcesses = data.processes
        .sort((a, b) => b.cpu_load - a.cpu_load || b.mem_load - a.mem_load)

    tbody.innerHTML = topProcesses.map(p => {
        const prioColor = p.priority < 0 ? 'text-red-400' : (p.priority > 0 ? 'text-blue-400' : 'text-emerald-300');
        return `
        <tr class="border-b border-emerald-800/50 hover:bg-emerald-800/30 transition-colors duration-150">
            <td class="p-3 text-emerald-400 font-mono text-sm">${p.id}</td>
            <td class="p-3 font-medium text-emerald-200">${p.user}</td>
            <td class="p-3 font-mono text-sm ${prioColor}">${p.priority}</td>
            <td class="p-3">
                <span class="px-2 py-1 rounded-md text-xs border font-mono ${getStateColor(p.state)}">
                    ${p.state}
                </span>
            </td>
            <td class="p-3 text-emerald-200 font-mono text-sm">${p.cpu_load.toFixed(1)}%</td>
            <td class="p-3 text-emerald-200 font-mono text-sm">${p.mem_load.toFixed(1)}%</td>
            <td class="p-3 text-emerald-400 font-mono text-sm">${formatTimePlus(p.execution_time)}</td>
            <td class="p-3 text-emerald-400 font-mono text-sm" title="Resident: ${formatBytes(p.resident_memory)}">
                ${formatBytes(p.resident_memory, 0)}
            </td>
            <td class="p-3 text-emerald-400 font-mono text-sm" title="Virtual: ${formatBytes(p.virtual_memory)}">
                ${formatBytes(p.virtual_memory, 0)}
            </td>
            <td class="p-3 text-emerald-200 max-w-xs truncate" title="${p.command}">
                <div class="truncate bg-emerald-950/50 px-2 py-1 rounded font-mono text-sm">${p.command}</div>
            </td>
        </tr>
    `}).join('');
}

const ws = new WebSocket(`ws://${window.location.host}`);

const statusEl = document.getElementById('connection-status')!;
const statusDot = document.getElementById('status-dot')!;

ws.onopen = () => {
    statusEl.textContent = 'Connected';
    statusEl.classList.replace('text-slate-300', 'text-green-400');
    statusDot.className = 'w-3 h-3 rounded-full bg-green-500 shadow-[0_0_8px_rgba(34,197,94,0.8)]';
};

ws.onclose = () => {
    statusEl.textContent = 'Disconnected';
    statusEl.classList.replace('text-green-400', 'text-red-400');
    statusDot.className = 'w-3 h-3 rounded-full bg-red-500 shadow-[0_0_8px_rgba(239,68,68,0.8)] animate-pulse';
};

ws.onerror = (error) => {
    console.error("WebSocket Error:", error);
};

ws.onmessage = (event) => {
    try {
        const data: SystemData = JSON.parse(event.data);
        console.log("Data from backend:", data);
        updateUI(data);
    } catch (e) {
        console.error("Failed to parse JSON from backend:", e);
    }
};