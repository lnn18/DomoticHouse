'use strict';

/**
 * Utilidad compartida para comparar pines declarados en el código fuente
 * (src/, include/, lib/) contra las conexiones definidas en diagram.json.
 * Usada por el comando /pin-check y por el hook pin-consistency-check.js.
 */

const fs = require('fs');
const path = require('path');

function extractDiagramPins(diagramJson) {
  const pins = new Set();
  const connections = (diagramJson && diagramJson.connections) || [];
  const walk = (node) => {
    if (Array.isArray(node)) {
      node.forEach(walk);
    } else if (typeof node === 'string') {
      const m = node.match(/^uno:(\d+)$/);
      if (m) pins.add(Number(m[1]));
    }
  };
  walk(connections);
  return pins;
}

function extractCodePins(sourceText) {
  const pins = new Map();
  const declRegex = /\b(?:int|const\s+int|constexpr\s+int|uint8_t|byte)\s+(\w*[Pp]in\w*)\s*=\s*(\d+)\s*;/g;
  const defineRegex = /#define\s+(\w*PIN\w*)\s+(\d+)/gi;
  let match;
  while ((match = declRegex.exec(sourceText)) !== null) {
    pins.set(match[1], Number(match[2]));
  }
  while ((match = defineRegex.exec(sourceText)) !== null) {
    pins.set(match[1], Number(match[2]));
  }
  return pins;
}

function findSourceFiles(root) {
  const dirs = ['src', 'include', 'lib'];
  const exts = new Set(['.cpp', '.h', '.hpp', '.cc']);
  const files = [];
  const walkDir = (dir) => {
    if (!fs.existsSync(dir)) return;
    for (const entry of fs.readdirSync(dir, { withFileTypes: true })) {
      const full = path.join(dir, entry.name);
      if (entry.isDirectory()) {
        walkDir(full);
      } else if (exts.has(path.extname(entry.name))) {
        files.push(full);
      }
    }
  };
  dirs.forEach((d) => walkDir(path.join(root, d)));
  return files;
}

function checkPinConsistency(root) {
  const diagramPath = path.join(root, 'diagram.json');
  if (!fs.existsSync(diagramPath)) {
    return { ok: true, warnings: ['No se encontró diagram.json en la raíz del proyecto — se omite la verificación.'] };
  }

  let diagramJson;
  try {
    diagramJson = JSON.parse(fs.readFileSync(diagramPath, 'utf8'));
  } catch (err) {
    return { ok: true, warnings: [`No se pudo parsear diagram.json: ${err.message}`] };
  }

  const diagramPins = extractDiagramPins(diagramJson);

  const codePinsByName = new Map();
  for (const file of findSourceFiles(root)) {
    const text = fs.readFileSync(file, 'utf8');
    for (const [name, pin] of extractCodePins(text)) {
      codePinsByName.set(name, { pin, file });
    }
  }

  const codePinNumbers = new Set([...codePinsByName.values()].map((v) => v.pin));

  const enCodigoNoEnDiagrama = [...codePinsByName.entries()]
    .filter(([, v]) => !diagramPins.has(v.pin))
    .map(([name, v]) => `${name} = ${v.pin} (${path.relative(root, v.file)})`);

  const enDiagramaNoEnCodigo = [...diagramPins]
    .filter((p) => !codePinNumbers.has(p))
    .map((p) => `uno:${p}`);

  return {
    ok: enCodigoNoEnDiagrama.length === 0 && enDiagramaNoEnCodigo.length === 0,
    enCodigoNoEnDiagrama,
    enDiagramaNoEnCodigo,
  };
}

module.exports = { extractDiagramPins, extractCodePins, findSourceFiles, checkPinConsistency };

if (require.main === module) {
  const root = process.cwd();
  const result = checkPinConsistency(root);

  (result.warnings || []).forEach((w) => console.log(w));

  if (result.enCodigoNoEnDiagrama && result.enCodigoNoEnDiagrama.length) {
    console.log('Pines usados en el código pero no conectados en diagram.json:');
    result.enCodigoNoEnDiagrama.forEach((line) => console.log(`  - ${line}`));
  }

  if (result.enDiagramaNoEnCodigo && result.enDiagramaNoEnCodigo.length) {
    console.log('Pines conectados en diagram.json pero no referenciados en el código:');
    result.enDiagramaNoEnCodigo.forEach((line) => console.log(`  - ${line}`));
  }

  if (result.ok) {
    console.log('Sincronía de pines: OK');
  }
}
