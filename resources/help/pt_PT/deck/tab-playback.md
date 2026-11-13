# Separador Reprodução

<!-- keywords: separador reprodução, friso, filme, vídeo, som, áudio, reproduzir, pausa, pré-visualizar, cortar, legendas, inserir legendas, remover legendas, subtítulos, webvtt, vtt, volume, silenciar, iniciar, na sequência de cliques, automaticamente, ao clicar, reproduzir em ecrã inteiro, ocultar quando não estiver a reproduzir, repetir até parar, recuar após a reprodução, ferramentas de multimédia -->

O separador **Reprodução** define a forma como um filme ou um som é reproduzido e acrescenta legendas a um filme. Só é apresentado enquanto um filme ou um som estiver selecionado, e desaparece quando selecionar outra coisa.

- **Pré-visualizar** tem **Reproduzir**. Reproduz o filme ou o som no diapositivo para que o possa ver e ouvir. Enquanto está a ser reproduzido, o botão diz **Pausa**.
- **Editar** tem **Cortar**. Abre uma caixa com uma **Hora de início** e uma **Hora de fim**, em segundos. O filme ou o som é reproduzido apenas entre a hora de início e a hora de fim. O restante ficheiro continua dentro da apresentação. Para voltar a reproduzir até ao fim do ficheiro, defina a hora de fim como a duração total.
  - **Inserir Legendas** acrescenta legendas a um filme a partir de um ficheiro WebVTT (um ficheiro de texto que termina em `.vtt`). Pede o ficheiro e, depois, o idioma das legendas, como um código curto, por exemplo `en`, `es` ou `pt-BR`. As legendas são associadas a um filme incorporado na apresentação; para um filme apenas ligado a partir do exterior, o botão fica esbatido. Um filme mantém um conjunto de legendas por cada idioma: se inserir um segundo ficheiro para o mesmo idioma, o primeiro é substituído. Um ficheiro que não seja WebVTT, que não contenha legendas ou que seja demasiado grande (o máximo é 4 MB, 20 000 legendas e 2000 carateres numa mesma legenda) é recusado com uma mensagem, e nada muda.
  - **Remover Legendas** retira as legendas do filme. Se o filme tiver legendas em mais do que um idioma, pergunta quais remover, ou se pretende remover todas.
- **Opções de Multimédia** indica como o filme ou o som é reproduzido na apresentação de diapositivos:
  - **Volume** é **Baixo**, **Médio**, **Alto** ou **Silenciar**.
  - **Iniciar** indica o que o inicia. **Na Sequência de Cliques** reproduz o filme ou o som ao clicar, em conjunto com as animações do diapositivo. **Automaticamente** reproduz o filme ou o som sozinho quando chega a sua vez. **Ao Clicar** reproduz o filme ou o som quando clicar no filme ou no som.
  - **Reproduzir em Ecrã Inteiro** faz com que um filme ocupe o ecrã enquanto é reproduzido. Funciona apenas para filmes.
  - **Ocultar Quando Não Estiver a Reproduzir** oculta a imagem até o filme ou o som ser reproduzido, e volta a ocultá-la depois de parar. O ícone de altifalante de um som fica sempre oculto.
  - **Repetir até Parar** reproduz de novo sempre que termina, até ser interrompido ou até o diapositivo mudar.
  - **Recuar Após a Reprodução** faz com que um filme volte à primeira imagem quando termina. Sem esta opção, o filme fica na última imagem.

As legendas são apresentadas na parte inferior do filme, em texto branco sobre uma faixa escura, enquanto o filme é reproduzido na apresentação de diapositivos e no editor. Estão sincronizadas com o filme, e o respetivo tamanho acompanha o tamanho do filme. Se um filme tiver mais do que um idioma, é apresentado o primeiro. As legendas são guardadas com a apresentação, como parte do ficheiro `.pptx`, pelo que voltam a aparecer quando a abrir novamente. Um ficheiro de legendas noutro formato (que não seja WebVTT) é mantido tal como está e é guardado de novo sem alterações, mas não é apresentado. Depois de um filme ter legendas, a **Verificar Acessibilidade** deixa de o indicar.

Cada alteração é um passo. **Ctrl+Z** anula-a.

Um som ou filme que é reproduzido através de vários diapositivos, como definem alguns ficheiros, continua a ser reproduzido durante o número de diapositivos indicado no ficheiro. O JeffDeck reproduz desta forma na apresentação de diapositivos e mantém esta definição ao guardar.

Consulte [Formas, imagens, tabelas e Texto Artístico](objects) e [Reproduzir a apresentação de diapositivos e usar a Vista de Apresentador](show).

## Relacionados

- [Formas, imagens, tabelas e Texto Artístico](objects)
- [Separador Inserir](tab-insert)
- [Reproduzir a apresentação de diapositivos e usar a Vista de Apresentador](show)
