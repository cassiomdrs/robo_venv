import io
import wave
import numpy as np
import speech_recognition as sr
from flask import Flask, request

app = Flask(__name__)
recognizer = sr.Recognizer()


@app.route("/transcribe", methods=["POST"])
def transcribe():
    raw_pcm = request.get_data()
    if not raw_pcm:
        return "", 400

    samples = np.frombuffer(raw_pcm, dtype=np.int16)

    wav_io = io.BytesIO()
    with wave.open(wav_io, "wb") as wf:
        wf.setnchannels(1)
        wf.setsampwidth(2)  # 16-bit
        wf.setframerate(16000)
        wf.writeframes(samples.tobytes())
    wav_io.seek(0)

    try:
        with sr.AudioFile(wav_io) as source:
            audio_data = recognizer.record(source)
        texto = recognizer.recognize_google(audio_data, language="pt-BR").strip()
    except sr.UnknownValueError:
        texto = ""
    except sr.RequestError as exc:
        print(f"Erro no servico de transcricao: {exc}")
        return "", 502

    print(f"Transcrito: {texto}")
    return texto, 200


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5005)
