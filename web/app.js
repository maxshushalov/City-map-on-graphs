let graphData = null;
let deliveryData = null;

// Функция загрузки графа
async function loadGraph() {
    try {
        const response = await fetch('graph.json');
        graphData = await response.json();
        //console.log('Граф загружен:', graphData);
        drawGraph(graphData);
    } catch (error) {
        alert('Ошибка: файл graph.json не найден!');
        console.error(error);
    }
}

function drawGraph(data) {
    const svg = document.getElementById('graphCanvas');
    svg.innerHTML = '';

    const xCoords = data.vertices.map(v => v.x);
    const yCoords = data.vertices.map(v => v.y);

    const minX = Math.min(...xCoords);
    const maxX = Math.max(...xCoords);
    const minY = Math.min(...yCoords);
    const maxY = Math.max(...yCoords);

    const padding = 100;
    const width = maxX - minX + padding * 2;
    const height = maxY - minY + padding * 2;

    svg.setAttribute('viewBox', `${minX - padding} ${minY - padding} ${width} ${height}`);

    const step = 150;
    const blockSize = step - 40;

    // Кварталы городской сетки
    for (let x = minX; x < maxX; x += step) {
        for (let y = minY; y < maxY; y += step) {
            const rect = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
            rect.setAttribute('x', x + 20);
            rect.setAttribute('y', y + 20);
            rect.setAttribute('width', blockSize);
            rect.setAttribute('height', blockSize);
            rect.setAttribute('class', 'block');
            svg.appendChild(rect);
        }
    }

    // Тротуары
    for (let x = minX; x <= maxX; x += step) {
        const line = document.createElementNS('http://www.w3.org/2000/svg', 'line');
        line.setAttribute('x1', x);
        line.setAttribute('y1', minY);
        line.setAttribute('x2', x);
        line.setAttribute('y2', maxY);
        line.setAttribute('class', 'sidewalk');
        svg.appendChild(line);
    }

    for (let y = minY; y <= maxY; y += step) {
        const line = document.createElementNS('http://www.w3.org/2000/svg', 'line');
        line.setAttribute('x1', minX);
        line.setAttribute('y1', y);
        line.setAttribute('x2', maxX);
        line.setAttribute('y2', y);
        line.setAttribute('class', 'sidewalk');
        svg.appendChild(line);
    }


    // ДОРОГИ с окраской по загруженности
    data.edges.forEach(edge => {
        const v1 = data.vertices.find(v => v.id === edge.from);
        const v2 = data.vertices.find(v => v.id === edge.to);

        const line = document.createElementNS('http://www.w3.org/2000/svg', 'line');
        line.setAttribute('x1', v1.x);
        line.setAttribute('y1', v1.y);
        line.setAttribute('x2', v2.x);
        line.setAttribute('y2', v2.y);
        line.setAttribute('data-from', edge.from);
        line.setAttribute('data-to', edge.to);

        let trafficColor;
        if (edge.blocked) {
            trafficColor = '#9E9E9E';  // Серый — заблокировано
        } else if (edge.trafficLevel >= 2.0) {
            trafficColor = '#FF6F00';  // Оранжевый — сильная пробка
        } else if (edge.trafficLevel > 1.0) {
            trafficColor = '#FFD600';  // Жёлтый — лёгкая пробка
        } else {
            trafficColor = '#4CAF50';  // Зелёный — свободная дорога
        }


        line.setAttribute('stroke', trafficColor);
        line.setAttribute('class', 'edge');
        line.setAttribute('stroke-width', 8);
        line.setAttribute('stroke-linecap', 'round');


        line.addEventListener('click', function(event) {
            event.stopPropagation();
            showEdgeMenu(edge.from, edge.to, event.clientX, event.clientY);
        });

        svg.appendChild(line);
    });


    // Вершины (перекрестки)
    data.vertices.forEach(vertex => {
        const g = document.createElementNS('http://www.w3.org/2000/svg', 'g');

        const circle = document.createElementNS('http://www.w3.org/2000/svg', 'circle');
        circle.setAttribute('cx', vertex.x);
        circle.setAttribute('cy', vertex.y);
        circle.setAttribute('r', 25);
        circle.setAttribute('class', 'vertex');
        circle.setAttribute('data-id', vertex.id);

        circle.addEventListener('click', function(event) {
            event.stopPropagation();
            showVertexMenu(vertex.id, event.clientX, event.clientY);
        });

        const text = document.createElementNS('http://www.w3.org/2000/svg', 'text');
        text.setAttribute('x', vertex.x);
        text.setAttribute('y', vertex.y + 5);
        text.textContent = vertex.id;
        text.setAttribute('class', 'label');

        g.appendChild(circle);
        g.appendChild(text);
        svg.appendChild(g);
    });

    if (deliveryData) {
        drawWarehouses(deliveryData);
    }
}

function drawWarehouses(data) {
    const svg = document.getElementById('graphCanvas');

    // Главный склад (фиолетовый квадрат)
    if (data.mainWarehouse !== undefined) {
        const mainVertex = graphData.vertices.find(v => v.id === data.mainWarehouse);
        if (mainVertex) {
            const rect = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
            rect.setAttribute('x', mainVertex.x - 20);
            rect.setAttribute('y', mainVertex.y - 20);
            rect.setAttribute('width', 40);
            rect.setAttribute('height', 40);
            rect.setAttribute('fill', '#9C27B0');
            rect.setAttribute('stroke', '#fff');
            rect.setAttribute('stroke-width', 3);
            rect.setAttribute('class', 'warehouse-main');
            svg.appendChild(rect);

            // Текст с номером
            const text = document.createElementNS('http://www.w3.org/2000/svg', 'text');
            text.setAttribute('x', mainVertex.x);
            text.setAttribute('y', mainVertex.y + 5);
            text.textContent = data.mainWarehouse;
            text.setAttribute('class', 'label');
            svg.appendChild(text);
        }
    }

    // Пункты доставки (синие треугольники)
    if (data.deliveryWarehouses && data.deliveryWarehouses.length > 0) {
        data.deliveryWarehouses.forEach(whId => {
            const vertex = graphData.vertices.find(v => v.id === whId);
            if (vertex) {
                const triangle = document.createElementNS('http://www.w3.org/2000/svg', 'polygon');
                const x = vertex.x;
                const y = vertex.y;
                triangle.setAttribute('points', `${x},${y-25} ${x-20},${y+15} ${x+20},${y+15}`);
                triangle.setAttribute('fill', '#2196F3');
                triangle.setAttribute('stroke', '#fff');
                triangle.setAttribute('stroke-width', 2);
                triangle.setAttribute('class', 'warehouse-delivery');
                svg.appendChild(triangle);

                // Текст с номером
                const text = document.createElementNS('http://www.w3.org/2000/svg', 'text');
                text.setAttribute('x', x);
                text.setAttribute('y', y + 5);
                text.textContent = whId;
                text.setAttribute('class', 'label');
                svg.appendChild(text);
            }
        });
    }
}

// Линии подсветки рисуются под дорогой
function highlightPath(pathVertices) {
    const svg = document.getElementById('graphCanvas');


    document.querySelectorAll('.path-underlay').forEach(el => el.remove());


    for (let i = 0; i < pathVertices.length - 1; i++) {
        const from = pathVertices[i];
        const to = pathVertices[i + 1];

        const v1 = graphData.vertices.find(v => v.id === from);
        const v2 = graphData.vertices.find(v => v.id === to);

        if (v1 && v2) {
            const redLine = document.createElementNS('http://www.w3.org/2000/svg', 'line');
            redLine.setAttribute('x1', v1.x);
            redLine.setAttribute('y1', v1.y);
            redLine.setAttribute('x2', v2.x);
            redLine.setAttribute('y2', v2.y);
            redLine.setAttribute('class', 'path-underlay'); // Новый класс
            redLine.setAttribute('stroke', '#FF0000');
            redLine.setAttribute('stroke-width', '20'); // ТОЛЩЕ дороги
            redLine.setAttribute('stroke-linecap', 'round');
            redLine.style.filter = 'drop-shadow(0 0 6px #FF0000)';


            svg.insertBefore(redLine, svg.firstChild);
        }
    }
}

function highlightPathNoCleanup(pathVertices, routeIndex) { // подсветка множества ммааршрутов без очистки старых
    const svg = document.getElementById('graphCanvas');

    // Цвета для разных маршрутов
    const colors = ['#FF0000', '#FF6B00', '#FF9500', '#FFB800', '#FFDB00'];
    const color = colors[routeIndex % colors.length];

    for (let i = 0; i < pathVertices.length - 1; i++) {
        const from = pathVertices[i];
        const to = pathVertices[i + 1];

        const v1 = graphData.vertices.find(v => v.id === from);
        const v2 = graphData.vertices.find(v => v.id === to);

        if (v1 && v2) {
            const line = document.createElementNS('http://www.w3.org/2000/svg', 'line');
            line.setAttribute('x1', v1.x);
            line.setAttribute('y1', v1.y);
            line.setAttribute('x2', v2.x);
            line.setAttribute('y2', v2.y);
            line.setAttribute('class', 'path-underlay');
            line.setAttribute('stroke', color);
            line.setAttribute('stroke-width', '16');
            line.setAttribute('stroke-linecap', 'round');
            line.setAttribute('opacity', '0.7');

            svg.insertBefore(line, svg.firstChild);
        }
    }
}

// кнопка загрузки графа
document.getElementById('loadGraph').addEventListener('click', loadGraph);

// кнопка поиска кратчайшего пути
document.getElementById('findPath').addEventListener('click', async function() {
    if (!graphData) {
        alert('Сначала загрузите граф!');
        return;
    }

    const start = parseInt(document.getElementById('startInput').value);
    const end = parseInt(document.getElementById('endInput').value);

    if (isNaN(start) || isNaN(end)) {
        alert('Введите корректные номера вершин!');
        return;
    }

    try {

        const response = await fetch(`/api/find?start=${start}&end=${end}`);
        const data = await response.json();

        if (!data.found) {
            alert(`Путь из ${start} в ${end} не найден!`);
            return;
        }


        deliveryData = null;
        drawGraph(graphData);


        highlightPath(data.path);

        alert(`Путь найден!\nВремя: ${data.totalTime.toFixed(2)} мин\nПерекрёстков: ${data.path.length}`);
        //console.log('Найденный путь:', data.path);

    } catch (error) {
        alert('Ошибка поиска пути: ' + error.message);
        console.error(error);
    }
});

// Кнопка генерации событий
document.getElementById('generateEvents').addEventListener('click', async function() {
    try {
        const response = await fetch(`/api/generate?hour=${hour}`);
        const data = await response.json();

        if (data.status === 'success') {
            await loadGraph();
            //alert('События сгенерированы, граф обновлён!');
        } else if (data.error) {
            alert('Ошибка генерации: ' + data.error);
        } else {
            alert('Неизвестный ответ от /api/generate');
        }
    } catch (error) {
        alert('Ошибка запроса /api/generate: ' + error.message);
        console.error(error);
    }
});

document.getElementById('buildRoute').addEventListener('click', async function() {
    if (!graphData) {
        alert('Сначала загрузите граф!');
        return;
    }

    try {
        // Строим маршрут доставки по текущим складам
        const response = await fetch('/api/delivery');
        deliveryData = await response.json();

        if (!deliveryData.fullPath && !deliveryData.segments) {
            alert('Не удалось построить маршрут');
            return;
        }

        drawGraph(graphData);

        document.querySelectorAll('.path-underlay').forEach(el => el.remove());

        if (deliveryData.fullPath && deliveryData.fullPath.length > 0) { // маршрут большой машины
            highlightPath(deliveryData.fullPath);
        } else if (deliveryData.segments && deliveryData.segments.length > 0) {
            deliveryData.segments.forEach((seg, index) => {
                if (seg.path && seg.path.length > 0) {
                    highlightPathNoCleanup(seg.path, index);
                }
            });
        }

        alert(`Маршрут построен!\nВремя: ${deliveryData.totalTime.toFixed(2)}
         мин\nУспешно: ${deliveryData.successCount}/${deliveryData.totalWarehouses}`);

    } catch (error) {
        alert('Ошибка построения маршрута: ' + error.message);
        console.error(error);
    }
});

// КНОПКА: Режим доставки
document.getElementById('deliveryMode').addEventListener('click', async function() {
    if (!graphData) {
        alert('Сначала загрузите граф!');
        return;
    }

    try {
        const response = await fetch('/api/delivery');

        deliveryData = await response.json();

        //console.log('Данные доставки:', deliveryData);

        if (!deliveryData.fullPath && !deliveryData.segments) {
            alert('Нет данных о маршруте доставки');
            return;
        }

        drawGraph(graphData);

        document.querySelectorAll('.path-underlay').forEach(el => el.remove());

        // Рисуем маршрут большой машины
        if (deliveryData.fullPath && deliveryData.fullPath.length > 0) {
            highlightPath(deliveryData.fullPath);
        } else if (deliveryData.segments && deliveryData.segments.length > 0) {
            deliveryData.segments.forEach((seg, index) => {
                if (seg.path && seg.path.length > 0) {
                    highlightPathNoCleanup(seg.path, index);
                }
            });
        }

        alert(`Режим доставки активирован!\n` +
            `Главный склад: ${deliveryData.mainWarehouse}\n` +
            `Пунктов доставки: ${deliveryData.deliveryWarehouses.length}\n` +
            `Успешных маршрутов: ${deliveryData.successCount}/${deliveryData.totalWarehouses}\n` +
            `Общее время: ${deliveryData.totalTime.toFixed(2)} мин\n` +
            `Вершин в маршруте: ${deliveryData.fullPath ? deliveryData.fullPath.length : 0}`);

    } catch (error) {
        alert('Ошибка загрузки режима доставки: ' + error.message);
        console.error(error);
    }
});


// Кнопка очистки подсветки
document.getElementById('clearHighlight').addEventListener('click', function() {
    document.querySelectorAll('.path-highlight').forEach(el => {
        el.classList.remove('path-highlight');
    });
    deliveryData = null;
    if (graphData) {
        drawGraph(graphData);
    }
});

document.getElementById('regenerateWarehouses').addEventListener('click', async function() {
    if (!graphData) {
        alert('Сначала загрузите граф!');
        return;
    }

    try {
        const response = await fetch('/api/regenerate?num=5');
        const data = await response.json();

        if (data.status === 'success') {
            deliveryData = {
                mainWarehouse: data.mainWarehouse,
                deliveryWarehouses: data.deliveryWarehouses
            };

            document.querySelectorAll('.path-underlay').forEach(el => el.remove());

            drawGraph(graphData);

            alert(`Склады перегенерированы!\nГлавный склад: ${data.mainWarehouse}\nПунктов доставки: ${data.deliveryWarehouses.length}\n\nТеперь нажмите "Построить маршрут"`);
        }
    } catch (error) {
        alert('Ошибка перегенерации: ' + error.message);
        console.error(error);
    }
});

// меню для вершины
function showVertexMenu(vertexId, x, y) {
    // Удаление старого меню
    const oldMenu = document.getElementById('vertexMenu');
    if (oldMenu) oldMenu.remove();

    // нвое меню
    const menu = document.createElement('div');
    menu.id = 'vertexMenu';
    menu.style.position = 'fixed';
    menu.style.left = x + 'px';
    menu.style.top = y + 'px';
    menu.style.background = '#fff';
    menu.style.border = '2px solid #333';
    menu.style.borderRadius = '8px';
    menu.style.padding = '10px';
    menu.style.zIndex = '10000';
    menu.style.boxShadow = '0 4px 12px rgba(0,0,0,0.3)';

    menu.innerHTML = `
        <div style="font-weight: bold; margin-bottom: 8px;">Вершина ${vertexId}</div>
        <button id="menuSetMain" style="width: 100%; margin: 4px 0; padding: 8px; cursor: pointer;">🟣 Главный склад</button>
        <button id="menuAddDelivery" style="width: 100%; margin: 4px 0; padding: 8px; cursor: pointer;">🔵 Добавить доставку</button>
        <button id="menuRemoveDelivery" style="width: 100%; margin: 4px 0; padding: 8px; cursor: pointer;">❌ Удалить доставку</button>
        <button id="menuClose" style="width: 100%; margin: 4px 0; padding: 8px; cursor: pointer;">Закрыть</button>
    `;

    document.body.appendChild(menu);

    document.getElementById('menuSetMain').onclick = () => {
        setMainWarehouse(vertexId);
        menu.remove();
    };

    document.getElementById('menuAddDelivery').onclick = () => {
        addDeliveryWarehouse(vertexId);
        menu.remove();
    };

    document.getElementById('menuRemoveDelivery').onclick = () => {
        removeDeliveryWarehouse(vertexId);
        menu.remove();
    };

    document.getElementById('menuClose').onclick = () => {
        menu.remove();
    };

    setTimeout(() => {
        document.addEventListener('click', function closeMenu(e) {
            if (!menu.contains(e.target)) {
                menu.remove();
                document.removeEventListener('click', closeMenu);
            }
        });
    }, 100);
}

// меню для ребра
function showEdgeMenu(from, to, clientX, clientY) {
    // Удаляем старое меню, если есть
    const old = document.getElementById('edgeMenu');
    if (old) old.remove();

    const menu = document.createElement('div');
    menu.id = 'edgeMenu';
    menu.style.position = 'fixed';
    menu.style.left = clientX + 'px';
    menu.style.top = clientY + 'px';
    menu.style.background = '#fff';
    menu.style.border = '1px solid #ccc';
    menu.style.padding = '8px';
    menu.style.borderRadius = '4px';
    menu.style.boxShadow = '0 2px 6px rgba(0,0,0,0.2)';
    menu.style.zIndex = 1000;
    menu.style.fontSize = '14px';

    menu.innerHTML = `
        <div style="margin-bottom:4px;"><b>Ребро ${from} ⇄ ${to}</b></div>
        <button data-mode="green">Зелёное (свободно)</button><br>
        <button data-mode="yellow">Жёлтое (средняя)</button><br>
        <button data-mode="orange">Оранжевое (сильная)</button><br>
        <button data-mode="block">Серое (блок)</button><br>
        <button data-mode="unblock">Снять блокировку</button>
    `;

    menu.querySelectorAll('button').forEach(btn => {
        btn.addEventListener('click', async () => {
            const mode = btn.getAttribute('data-mode');
            menu.remove();
            await applyEdgeMode(from, to, mode);
        });
    });

    document.body.appendChild(menu);

    const closeHandler = (e) => {
        if (!menu.contains(e.target)) {
            menu.remove();
            document.removeEventListener('click', closeHandler);
        }
    };
    setTimeout(() => {
        document.addEventListener('click', closeHandler);
    }, 0);
}



async function setMainWarehouse(vertexId) {
    try {
        const response = await fetch(`/api/set_main?vertex=${vertexId}`);
        const data = await response.json();

        if (data.status === 'success') {
            deliveryData = {
                mainWarehouse: data.mainWarehouse,
                deliveryWarehouses: data.deliveryWarehouses,
                routes: []
            };
            drawGraph(graphData);
            //alert(`Вершина ${vertexId} назначена главным складом!`);
        }
    } catch (error) {
        alert('Ошибка: ' + error.message);
    }
}

async function addDeliveryWarehouse(vertexId) {
    try {
        const response = await fetch(`/api/add_delivery?vertex=${vertexId}`);
        const data = await response.json();

        if (data.status === 'success') {
            deliveryData = {
                mainWarehouse: data.mainWarehouse,
                deliveryWarehouses: data.deliveryWarehouses,
                routes: []
            };
            drawGraph(graphData);
            alert(`Вершина ${vertexId} добавлена в пункты доставки!`);
        }
    } catch (error) {
        alert('Ошибка: ' + error.message);
    }
}


async function removeDeliveryWarehouse(vertexId) {
    try {
        const response = await fetch(`/api/remove_delivery?vertex=${vertexId}`);
        const data = await response.json();

        if (data.status === 'success') {
            deliveryData = {
                mainWarehouse: data.mainWarehouse,
                deliveryWarehouses: data.deliveryWarehouses,
                routes: []
            };
            drawGraph(graphData);
            alert(`Вершина ${vertexId} удалена из пунктов доставки!`);
        }
    } catch (error) {
        alert('Ошибка: ' + error.message);
    }
}

async function onEdgeClick(event) {
    event.stopPropagation();
    const line = event.target;
    const from = parseInt(line.getAttribute('data-from'));
    const to   = parseInt(line.getAttribute('data-to'));

    const mode = prompt(
        `Изменить ребро ${from} ⇄ ${to}.\n` +
        `Варианты:\n` +
        `  green   - зелёная\n` +
        `  yellow  - жёлтая\n` +
        `  orange  - оранжевая\n` +
        `  block   - заблокировать\n` +
        `  unblock - снять блокировку\n\n` +
        `Введите режим:`,
        'yellow'
    );

    if (!mode) return;

    const valid = ['green', 'yellow', 'orange', 'block', 'unblock'];
    if (!valid.includes(mode)) {
        alert('Неверный режим');
        return;
    }

    try {
        const response = await fetch(`/api/edit_edge?from=${from}&to=${to}&mode=${mode}`);
        const data = await response.json();

        if (data.status !== 'success') {
            alert('Ошибка изменения ребра: ' + (data.message || 'unknown'));
            console.error(data);
            return;
        }

        await loadGraph();
    } catch (err) {
        alert('Ошибка /api/edit_edge: ' + err.message);
        console.error(err);
    }
}

async function applyEdgeMode(from, to, mode) {
    try {
        const response = await fetch(`/api/edit_edge?from=${from}&to=${to}&mode=${mode}`);
        const data = await response.json();

        if (data.status !== 'success') {
            alert('Ошибка изменения ребра: ' + (data.message || 'unknown'));
            console.error(data);
            return;
        }

        // Перечитываем graph.json и перерисовываем
        await loadGraph();
    } catch (err) {
        alert('Ошибка /api/edit_edge: ' + err.message);
        console.error(err);
    }
}
