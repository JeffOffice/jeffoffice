# Guia Reprodução
<!-- keywords: guia reprodução, faixa de opções, filme, vídeo, som, áudio, reproduzir, pausar, visualizar, cortar, legendas, inserir legendas, remover legendas, subtítulos, webvtt, vtt, volume, silenciar, iniciar, em sequência de cliques, automaticamente, ao clicar, reproduzir em tela cheia, ocultar quando não estiver em reprodução, repetir até ser interrompido, retroceder após reproduzir, ferramentas de mídia -->

A guia **Reprodução** define como um filme ou um som é reproduzido e adiciona legendas a um filme. Ela aparece somente enquanto um filme ou um som está selecionado, e desaparece quando você seleciona outra coisa.

- **Visualizar** tem **Reproduzir**. Ele reproduz o filme ou o som no slide para que você possa ver e ouvir. Enquanto ele é reproduzido, o botão mostra **Pausar**.
- **Edição** tem **Cortar**. Ele abre uma caixa com **Hora de início** e **Hora de término**, em segundos. O filme ou o som é reproduzido somente do tempo inicial até o tempo final. O restante do arquivo permanece dentro da apresentação. Para reproduzir até o fim do arquivo novamente, defina a hora de término como a duração total.
  - **Inserir Legendas** adiciona legendas a um filme a partir de um arquivo WebVTT (um arquivo de texto que termina em `.vtt`). Ele pede o arquivo e depois o idioma das legendas, como um código curto, por exemplo `en`, `es` ou `pt-BR`. As legendas são aplicadas a um filme incorporado à apresentação; para um filme que está apenas vinculado de fora, o botão fica desativado. Um filme mantém um conjunto de legendas para cada idioma: inserir um segundo arquivo para o mesmo idioma substitui o primeiro. Um arquivo que não é WebVTT, não contém legendas ou é grande demais (o máximo é 4 MB, 20.000 legendas e 2.000 caracteres em uma única legenda) é recusado com uma mensagem, e nada muda.
  - **Remover Legendas** retira as legendas do filme. Se o filme tiver legendas em mais de um idioma, ele pergunta quais remover ou se devem ser removidas todas.
- **Opções de Mídia** define como ele é reproduzido na apresentação de slides:
  - **Volume** é **Baixo**, **Médio**, **Alto** ou **Silenciar**.
  - **Iniciar** define o que o inicia. **Em Sequência de Cliques** reproduz o conteúdo em um clique, junto com as animações do slide. **Automaticamente** reproduz o conteúdo sozinho quando chega a sua vez. **Ao Clicar** reproduz o conteúdo quando você clica no filme ou no som.
  - **Reproduzir em Tela Cheia** faz um filme ocupar a tela inteira enquanto é reproduzido. Funciona somente para filmes.
  - **Ocultar Quando Não Estiver em Reprodução** oculta a imagem até que o filme ou o som seja reproduzido, e novamente depois que ele para. A imagem do alto-falante de um som fica oculta o tempo todo.
  - **Repetir até Ser Interrompido** reproduz o conteúdo novamente sempre que ele termina, até que seja interrompido ou o slide mude.
  - **Retroceder Após Reproduzir** faz um filme voltar à sua primeira imagem quando termina. Sem essa opção, o filme permanece na última imagem.

As legendas aparecem na parte inferior do filme, em texto branco sobre uma faixa escura, enquanto o filme é reproduzido na apresentação de slides e no editor. Elas são sincronizadas com o filme, e o tamanho delas acompanha o tamanho do filme. Se um filme tiver mais de um idioma, o primeiro é exibido. As legendas são salvas com a apresentação, como parte do arquivo `.pptx`, e voltam quando você o abre novamente. Um arquivo de legenda em outro formato (que não seja WebVTT) é mantido como está e é salvo novamente sem alterações, mas não aparece. Quando um filme tem legendas, **Verificar Acessibilidade** não o aponta mais.

Cada alteração é uma etapa. **Ctrl+Z** a desfaz.

Um som ou filme que é reproduzido através de vários slides, como alguns arquivos definem, continua tocando pelo número de slides indicado no arquivo. O JeffDeck reproduz desse modo na apresentação de slides e mantém essa configuração ao salvar.

Veja [Formas, imagens, tabelas e Texto Artístico](objects) e [Reproduzir a apresentação de slides e usar o Modo de Exibição do Apresentador](show).

## Relacionados

- [Formas, imagens, tabelas e Texto Artístico](objects)
- [Guia Inserir](tab-insert)
- [Reproduzir a apresentação de slides e usar o Modo de Exibição do Apresentador](show)
