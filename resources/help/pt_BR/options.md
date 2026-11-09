# Alterar as configurações do JeffPub
<!-- keywords: opções, configurações, preferências, tema, modo escuro, unidades, polegadas, recuperação automática, backup, idioma, nome do usuário, deslocar, atualizações, beta, estatísticas de uso, sobre -->

As suas configurações estão na janela **Opções do JeffPub**. Clique em **Arquivo > Opções** para abri-la. Ela tem quatro guias. Clique em **OK** para manter as alterações ou em **Cancelar** para descartá-las.

## Geral

- **Nome do usuário** e **Iniciais** são salvos como autor das novas publicações e aparecem nos comentários.
- **Tema** pode ser **Usar configuração do sistema**, **Claro** ou **Escuro**. A alteração aparece imediatamente.
- **Idioma de exibição** é o idioma dos textos do próprio JeffPub. **Usar idioma do sistema** segue o idioma do seu computador. Outros idiomas aparecem nesta lista quando há traduções disponíveis. Um novo idioma começa a valer na próxima vez que você abrir o JeffPub.
- **Enviar estatísticas de uso anônimas** e **Verificar atualizações ao iniciar o JeffPub** são as mesmas opções da página **Sobre** (veja abaixo).

**Nome do usuário**, **Iniciais**, **Tema** e **Idioma de exibição**, bem como as opções de atualização e de estatísticas, valem para todos os aplicativos do JeffOffice: se você alterar uma delas aqui, ela muda em todos os outros.

## Revisão de Texto

Estas caixas de seleção controlam a verificação ortográfica e a ajuda para digitação. Veja [Verificar ortografia e hifenização](proofing).

- **Verificar ortografia ao digitar**
- **Ignorar palavras em MAIÚSCULAS**
- **Ignorar palavras que contêm números**
- **Substituir texto ao digitar (AutoCorreção)** corrige erros comuns de digitação.
- **Substituir aspas retas por aspas inglesas** troca " e ' por aspas curvas.
- **Formatar automaticamente ao digitar (travessões, frações, ordinais, listas automáticas)**
- **Transformar endereços da web em links conforme você digita** transforma um endereço da web ou de email em link quando você pressiona a barra de espaço depois dele. Desfazer remove apenas o link.

## Salvar

- **Salvar informações de recuperação automática a cada** define com que frequência o JeffPub cria cópias de reserva do trabalho não salvo, de 1 a 120 minutos. Veja [Recuperar trabalho não salvo](recover).
- **Sempre criar cópia de backup** mantém o arquivo anterior com o nome "Backup de" seguido do nome do arquivo, a cada vez que você salvar.

## Avançado

- **Unidades de medida** podem ser **Polegadas**, **Centímetros**, **Milímetros**, **Pontos** ou **Paicas**. Elas definem as réguas e todas as caixas de medida. Até que você escolha, o JeffPub usa polegadas se a região do seu computador medir em polegadas, como nos Estados Unidos, e centímetros nos demais lugares. Um ponto equivale a 1/72 de polegada, e uma paica a 12 pontos.
- **As teclas de seta deslocam objetos em** define a distância que uma tecla de seta move um objeto selecionado.
- **Mostrar este número de publicações recentes** é quantos nomes a página **Abrir** lista, de 0 a 50.
- **Ao selecionar, selecionar automaticamente a palavra inteira** seleciona palavras inteiras ao arrastar o mouse sobre o texto.
- **Permitir arrastar e soltar texto** permite mover o texto selecionado arrastando-o.
- **Hifenizar automaticamente em novas caixas de texto** ativa a hifenização, que divide palavras longas no fim de uma linha, nas caixas de texto que você adicionar.

## Atualizações, estatísticas e seu nome

Clique em **Arquivo > Sobre** para encontrar estas opções. As duas caixas de seleção também estão na guia **Geral** de **Arquivo > Opções**, junto com uma terceira, **Obter versões beta**.

- **Verificar atualizações** procura agora uma versão mais recente. Se houver uma, a janela **Atualização disponível** mostra as novidades. Escolha **Atualizar agora**, **Mais tarde** ou **Ignorar esta versão**.
- **Obter versões beta (compilações antecipadas para teste)** faz o JeffPub também oferecer versões beta, que são lançadas antes das versões normais e podem ter imperfeições.
- **Verificar atualizações automaticamente** faz o JeffPub verificar sempre que for iniciado.
- **Enviar estatísticas de uso anônimas** é uma escolha sua sobre compartilhar informações sobre como o JeffPub é usado. Quando está ativada, o JeffPub envia estas informações uma vez por dia e no dia em que você atualizar: a versão, o sistema operacional e o idioma, e a frequência de uso de cada comando. Se o JeffPub fechar inesperadamente, ele também envia um relatório de falha na próxima vez que for iniciado. O relatório indica em que parte do programa ele parou, para que o problema possa ser encontrado e corrigido. Nunca são enviados seus arquivos, os nomes deles, nem qualquer conteúdo ou texto que você tenha digitado. Clique em **O que é enviado** para ler mais. O JeffPub pergunta sobre isso na primeira vez que é iniciado, em uma janela chamada **Ajude a melhorar o JeffPub**. No Windows, a instalação pode perguntar antes. A escolha vale para todos os aplicativos do JeffOffice: o JeffDeck envia o mesmo tipo de informação sobre si mesmo, somente se você tiver optado por compartilhar.
- **Aviso de privacidade**, ao lado dessa caixa de seleção, abre nosso aviso de privacidade em jeffoffice.com/privacy. Ele explica o que é enviado, para onde vai e por quanto tempo guardamos os dados. Mantemos as estatísticas por 24 meses.
- **Número de instalação** é exibido abaixo da caixa de seleção. É um número aleatório gerado para a sua cópia do JeffOffice, igual em todos os aplicativos do JeffOffice. Ele não revela nada sobre você. Clique em **Copiar** para copiá-lo. Se as estatísticas estiverem desativadas, ou se nenhum número tiver sido gerado ainda, aparece **nenhum**.
- Para excluir as estatísticas que já temos, envie o seu número de instalação para info@jeffoffice.com **antes** de desativar as estatísticas. Ao desativá-las, o número é apagado do seu computador e, sem ele, não conseguimos identificar quais registros são seus.
- Para desativar as estatísticas definitivamente, sem abrir o JeffPub, defina a variável de ambiente `JP_NO_TELEMETRY` com qualquer valor antes de iniciá-lo. Assim, nada é contado nem enviado, e o JeffPub não faz a pergunta da primeira execução.
- **Seu nome** é o mesmo que **Nome do usuário**, descrito acima.

## Relacionados

- [Recuperar trabalho não salvo](recover)
- [Verificar ortografia e hifenização](proofing)
- [Obter ajuda e relatar problemas](support)
