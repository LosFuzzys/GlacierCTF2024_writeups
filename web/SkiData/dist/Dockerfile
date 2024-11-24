FROM python:3.12-bookworm
EXPOSE 5001

RUN mkdir /app
WORKDIR /app

COPY . /app
RUN pip install -r requirements.txt

CMD ["python", "app.py"]