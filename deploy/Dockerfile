# Production Dockerfile — static web + health check
FROM nginx:1.27-alpine
LABEL maintainer="cricket-aaa" version="1.0.0"

# Security: non-root, no secrets in image
RUN adduser -D -H -u 1001 app && \
    apk add --no-cache curl

COPY public/ /usr/share/nginx/html/
COPY Playable/ /usr/share/nginx/html/playable/
COPY deploy/nginx.conf /etc/nginx/conf.d/default.conf
COPY deploy/security-headers.conf /etc/nginx/conf.d/security-headers.conf

HEALTHCHECK --interval=30s --timeout=3s --retries=3 CMD curl -f http://localhost/health.json || exit 1
EXPOSE 80
USER app
