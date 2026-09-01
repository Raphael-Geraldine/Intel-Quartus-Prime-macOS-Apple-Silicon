# Informações relevantes acerca da gravação no macOS em placas DE10-Lite

> Até o momento, a última versão do `openFPGAloader` só aceita alguns tipos de arquivos, como `.pof` e `.svf`; entretanto arquivos tradicionais `.sof` ainda não foram decodificados pelo `openFPGAloader` para a DE10-Lite, pois
> contêm certos padrões e metadados proprietários da Intel/Altera.

Para replicar o efeito dos arquivos `.sof`, que são voláteis e salvos exclusivamente na _SRAM_ sem apagar a memória _Flash_, podem ser utilizados arquivos `.svf` com as modificações destacadas a seguir:

## Instalação do openFPGAloader

Primeiramente, instale e descompacte o arquivo `.zip` na aba de [Releases](https://github.com/Raphael-Geraldine/Intel-Quartus-Prime-macOS-Apple-Silicon/releases/tag/DE10-Lite), ele contém uma versão 
modificada do código fonte (mantendo os termos da licença original) dessa ferramenta. Após isso, cole os scripts abaixo no terminal, que irão **remover a versão ativa atual** do `openFPGAloader` do seu macOS (caso exista) e instalar as dependências necessárias:

```zsh
brew uninstall -f openfpgaloader
brew install --only-dependencies openfpgaloader
brew install cmake pkg-config zlib
```

Agora, entre na pasta, do macOS, onde o `.zip` foi descompactado (ajuste o nome se necessário)

```zsh
cd ~/Downloads/openFPGALoader
```

E compile e instale a nova versão, no macOS:

```zsh
mkdir build && cd build
cmake ..
make -j$(sysctl -n hw.ncpu)
sudo make install
```

## Como gerar um arquivo `.svf`

1º) Após ter um projeto pronto e compilado em `.sof`, **no Quartus**, vá até “Tools > Programmer > Add file...” e adicione o `.sof` que deveria ir para a placa;

2º) Vá em “File > Create JAM, JBC, SVF or ISC File…”; escolha o `.svf` e mude para 24.0 MHz;

3º) Agora é só dar um "ok" e ir passar o arquivo para o macOS pela pasta compartilhada.

<p align="center">
  <img width="707" height="555" alt="svf" src="https://github.com/user-attachments/assets/f693064e-027c-4369-9d81-9dc1e9941c8e" />
</p>

## Modificação importante no arquivo

Por padrão, os arquivos `.svf` contêm instruções de limpar a memória _Flash_ ao serem subidos 
na placa. Para rodar o circuito direto na SRAM e manter a _Flash_ intacta (exatamente como o `.sof` faz):

1º) Abra o arquivo `.svf` no TextEdit.

2º) Apague o bloco que começa em `!Max 10 DSM Clear` e vai até o final de `!Max 10 DSM Verify` (logo antes de `!Max 10 Disable ISP`, deve ficar semelhante à imagem abaixo).

3º) Salve e suba o `.svf` normalmente.

![SVFModification](https://github.com/user-attachments/assets/5b40a98e-6594-40a0-bc1c-7e7d15c25267)

## Subindo o `.svf` para a DE10-Lite

Basta colar/digitar isto no terminal do macOS:

```zsh
openFPGALoader -c usb-blaster -m caminho/para/arquivo.svf
```

### O que fazer se esquecer a modificação

Neste cenário, o arquivo contido na memória _Flash_ terá sido apagado. Caso queira retornar ao arquivo padrão de fábrica, é possível baixá-lo do site oficial da [Terasic](https://download.terasic.com/downloads/cd-rom/de10-lite/DE10-Lite_v.2.2.0_SystemCD.zip). 
O `.pof` a ser subido novamente tem o seguinte caminho: `"DE10-Lite_v.2.2.0_SystemCD/Demonstrations/Default/DE10_LITE_Default.pof"`. Para subir na placa, basta colar/digitar isto no terminal do macOS:

```zsh
openFPGALoader -c usb-blaster -f caminho/para/DE10_LITE_Default.pof
```
