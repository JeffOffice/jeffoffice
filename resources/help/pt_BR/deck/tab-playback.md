# Guia Reprodução

<!-- keywords: guia reprodução, faixa de opções, filme, vídeo, som, áudio, reproduzir, pausar, visualizar, cortar, legendas, inserir legendas, remover legendas, legendas ocultas, webvtt, vtt, volume, silenciar, iniciar, na sequência de cliques, automaticamente, ao clicar, reproduzir em tela inteira, ocultar quando não estiver em reprodução, repetir até ser interrompido, retroceder após a reprodução, reproduzir entre slides, texto alternativo, alinhar, altura, largura, tamanho, ferramentas de mídia -->

A guia **Reprodução** define como um filme ou um som é reproduzido e adiciona legendas a um filme. Ela aparece apenas enquanto um filme ou um som está selecionado, e desaparece quando você seleciona outra coisa.

- **Visualizar** tem **Reproduzir**. Ele reproduz o filme ou o som no slide para que você possa vê-lo e ouvi-lo. Enquanto ele é reproduzido, o botão mostra **Pausar**.
- **Edição** tem **Cortar**. Ele abre uma caixa com **Hora de início** e **Hora de término**, em segundos. O filme ou o som é reproduzido somente do tempo inicial ao tempo final. O restante do arquivo permanece dentro da apresentação. Para reproduzir até o fim do arquivo novamente, defina a hora de término como a duração total.
  - **Inserir Legendas** adiciona legendas a um filme a partir de um arquivo WebVTT (um arquivo de texto com extensão `.vtt`). Ele pede o arquivo e, em seguida, o idioma das legendas, como um código curto, por exemplo `en`, `es` ou `pt-BR`. As legendas acompanham um filme que está incorporado à apresentação; para um filme apenas vinculado de fora, o botão fica desativado. Um filme mantém um conjunto de legendas para cada idioma: inserir um segundo arquivo para o mesmo idioma substitui o primeiro. Um arquivo que não seja WebVTT, que não contenha legendas ou que seja grande demais (o máximo é 4 MB, 20.000 legendas e 2.000 caracteres em uma legenda) é recusado com uma mensagem, e nada muda.
  - **Remover Legendas** retira as legendas do filme. Se o filme tiver legendas em mais de um idioma, ele pergunta quais remover, ou se devem ser removidas todas.
- **Opções de Mídia** define como ele é reproduzido na apresentação de slides:
  - **Volume** pode ser **Baixo**, **Médio**, **Alto** ou **Silenciar**.
  - **Iniciar** define o que o inicia. **Na Sequência de Cliques** reproduz o conteúdo em um clique, junto com as animações do slide. **Automaticamente** reproduz o conteúdo sozinho quando chega a sua vez. **Ao Clicar** reproduz o conteúdo quando você clica no filme ou no som.
  - **Reproduzir em Tela Inteira** faz um filme ocupar a tela inteira enquanto é reproduzido. Funciona apenas para filmes.
  - **Ocultar Quando Não Estiver em Reprodução** oculta a imagem até que o filme ou o som seja reproduzido, e novamente depois que ele para. A imagem de alto-falante de um som fica oculta o tempo todo.
  - **Repetir até Ser Interrompido** reproduz o conteúdo novamente sempre que ele termina, até que seja interrompido ou o slide mude.
  - **Retroceder Após a Reprodução** faz um filme voltar à sua primeira imagem quando termina. Sem essa opção, o filme permanece na última imagem.
  - **Reproduzir Entre Slides** (para um som; um filme é desenhado somente no próprio slide) mantém o som tocando na apresentação de slides quando o slide muda, até que termine ou seja interrompido, até o fim da apresentação. Um arquivo de outro programa pode indicar em quantos slides o som é reproduzido: esse número permanece enquanto a caixa estiver marcada, e o som para após essa quantidade de slides, contando a partir do slide em que começou (voltar e avançar entre eles não consome nenhum). Desmarcada, o som para junto com o seu slide; marcada novamente, ele continua até o fim da apresentação. Com **Repetir até Ser Interrompido**, ele recomeça sempre que termina. Ctrl+Z desfaz a alteração, e um arquivo .pptx mantém essa configuração.

As legendas aparecem sobre a parte inferior do filme, em texto branco sobre uma faixa escura, enquanto o filme é reproduzido na apresentação de slides e no editor. Elas são sincronizadas com o filme, e seu tamanho acompanha o tamanho do filme. Se um filme tiver mais de um idioma, o primeiro é exibido. As legendas são salvas com a apresentação, como parte do arquivo `.pptx`, e voltam quando você a abre novamente. Um arquivo de legendas em outro formato (que não seja WebVTT) é mantido como está e é salvo novamente sem alterações, mas não é exibido. Quando um filme tem legendas, **Verificar Acessibilidade** não o aponta mais.

Os grupos **Acessibilidade**, **Organizar** e **Tamanho** são os mesmos da guia Formato da Imagem: **Texto Alternativo**, **Alinhar** e as caixas **Altura** e **Largura**, para o filme ou som selecionado.

Cada alteração é uma etapa. **Ctrl+Z** a desfaz.

Um som ou filme que é reproduzido entre slides, como alguns arquivos definem, continua tocando pelo número de slides que o arquivo indica. O JeffDeck o reproduz dessa forma na apresentação de slides e mantém a configuração ao salvar.

Consulte [Formas, imagens, tabelas e Texto Artístico](objects) e [Reproduzir a apresentação de slides e usar o Modo de Exibição do Apresentador](show).

## Relacionados

- [Formas, imagens, tabelas e Texto Artístico](objects)
- [Guia Inserir](tab-insert)
- [Reproduzir a apresentação de slides e usar o Modo de Exibição do Apresentador](show)
