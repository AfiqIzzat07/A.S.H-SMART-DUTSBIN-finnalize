const fullnessText = document.getElementById('fullness-text');
const fill = document.getElementById('fill');
const statusDiv = document.getElementById('status');

const openBtn = document.getElementById('open-btn');
const closeBtn = document.getElementById('close-btn');

const servoRef = database.ref('/dustbin/servo');
const fullnessRef = database.ref('/dustbin/fullness');

function updateCircle(percent) {
  fullnessText.innerText = percent + '%';
  fill.style.transform = `rotate(${(percent / 100) * 180}deg)`;
}

fullnessRef.on('value', snapshot => {
  const percent = snapshot.val() || 0;
  updateCircle(percent);

  if (percent >= 80) statusDiv.innerText = '🚨 FULL';
  else if (percent <= 20) statusDiv.innerText = '✅ CLEANED';
  else statusDiv.innerText = '🟢 OK';
});

openBtn.addEventListener('click', () => servoRef.set('OPEN'));
closeBtn.addEventListener('click', () => servoRef.set('CLOSE'));

window.addEventListener('load', () => {
  if ('serviceWorker' in navigator) {
    navigator.serviceWorker.register('service-worker.js');
  }
});
