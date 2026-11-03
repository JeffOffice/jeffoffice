# Alterar as definições do JeffPub
<!-- keywords: opções, definições, preferências, configurações, tema, modo escuro, unidades, polegadas, centímetros, recuperação automática, cópia de segurança, idioma, nome de utilizador, deslocar, atualizações, beta, estatísticas de utilização, acerca de -->

As suas definições encontram-se na janela **Opções do JeffPub**. Clique em **Ficheiro > Opções** para a abrir. A janela tem quatro separadores. Clique em **OK** para manter as alterações ou em **Cancelar** para as descartar.

## Geral

- **Nome de utilizador** e **Iniciais** são guardados como autor das novas publicações e apresentados nos comentários.
- **Tema** pode ser **Utilizar definição do sistema**, **Claro** ou **Escuro**. A alteração é aplicada de imediato.
- **Idioma de apresentação** é o idioma dos textos do próprio JeffPub. **Utilizar idioma do sistema** segue o idioma do computador. Outros idiomas aparecem nesta lista quando existem traduções disponíveis. Um novo idioma passa a ser utilizado da próxima vez que abrir o JeffPub.
- **Enviar estatísticas de utilização anónimas** e **Procurar atualizações quando o JeffPub é iniciado** são as mesmas opções que se encontram na página **Acerca de** (ver abaixo).

**Nome de utilizador**, **Iniciais**, **Tema** e **Idioma de apresentação**, bem como as opções de atualizações e estatísticas, são comuns a todas as aplicações JeffOffice: se alterar uma aqui, ela é alterada em todas.

## Verificação

Estas caixas de verificação controlam a ajuda à ortografia e à escrita. Consulte [Verificar a ortografia e a hifenização](proofing).

- **Verificar a ortografia ao escrever**
- **Ignorar palavras em MAIÚSCULAS**
- **Ignorar palavras que contêm números**
- **Substituir texto ao escrever (Correção Automática)** corrige erros de escrita comuns.
- **Substituir aspas retas por aspas inteligentes** transforma " e ' em aspas curvas.
- **Formatação automática ao escrever (travessões, frações, ordinais, listas automáticas)**

## Guardar

- **Guardar informações de recuperação automática a cada** define com que frequência o JeffPub cria cópias de reserva do trabalho não guardado, de 1 a 120 minutos. Consulte [Recuperar trabalho não guardado](recover).
- **Criar sempre cópia de segurança** mantém o ficheiro anterior com o nome "Cópia de segurança de" seguido do nome do ficheiro, sempre que guarda.

## Avançadas

- **Unidades de medida** podem ser **Polegadas**, **Centímetros**, **Milímetros**, **Pontos** ou **Paicas**. Definem as réguas e todas as caixas de medidas. Enquanto não escolher, o JeffPub utiliza polegadas se a região do computador medir em polegadas, como nos Estados Unidos, e centímetros em todos os outros casos. Um ponto corresponde a 1/72 de polegada e uma paica a 12 pontos.
- **As teclas de seta deslocam os objetos em** define a distância que uma tecla de seta desloca um objeto selecionado.
- **Mostrar este número de Publicações Recentes** indica quantos nomes a página **Abrir** apresenta, de 0 a 50.
- **Ao selecionar, selecionar automaticamente a palavra inteira** seleciona palavras inteiras quando arrasta sobre o texto.
- **Permitir arrastar e largar texto** permite mover o texto selecionado arrastando-o.
- **Hifenizar automaticamente em novas caixas de texto** ativa a hifenização, que divide palavras longas no fim de uma linha, nas caixas de texto que adicionar.

## Atualizações, estatísticas e o seu nome

Clique em **Ficheiro > Acerca de** para encontrar estas opções. As duas caixas de verificação estão também no separador **Geral** de **Ficheiro > Opções**, juntamente com uma terceira, **Obter versões beta**.

- **Procurar Atualizações** procura de imediato uma versão mais recente. Se existir uma, a janela **Atualização Disponível** mostra as novidades. Escolha **Atualizar Agora**, **Mais Tarde** ou **Ignorar Esta Versão**.
- **Obter versões beta (compilações antecipadas para testes)** faz com que o JeffPub ofereça também versões beta, que saem antes das versões normais e podem ter algumas imperfeições.
- **Procurar atualizações automaticamente** faz com que o JeffPub procure atualizações sempre que é iniciado.
- **Enviar estatísticas de utilização anónimas** é a sua opção quanto à partilha de dados sobre a forma como o JeffPub é utilizado. Quando está ativada, o JeffPub envia estes dados uma vez por dia e no dia em que atualiza: a versão, o sistema operativo e o idioma, e a frequência com que cada comando é utilizado. Se o JeffPub alguma vez fechar inesperadamente, envia também um relatório de falha da próxima vez que for iniciado. O relatório indica o ponto do programa em que parou, para que o problema possa ser encontrado e corrigido. Nunca envia os seus ficheiros, os respetivos nomes ou o seu conteúdo, nem nada do que escreveu. Clique em **O que é enviado** para saber mais. O JeffPub pergunta-lhe sobre isto da primeira vez que é iniciado, numa janela chamada **Ajude a melhorar o JeffPub**. No Windows, a instalação pode perguntar primeiro. A opção aplica-se a todas as aplicações JeffOffice: o JeffDeck envia o mesmo tipo de dados sobre si próprio, apenas se tiver optado por partilhar.
- **Aviso de privacidade**, junto a essa caixa de verificação, abre o nosso aviso de privacidade em jeffoffice.com/privacy. Indica o que é enviado, para onde vai e durante quanto tempo o guardamos. Guardamos as estatísticas durante 24 meses.
- **Número de instalação** é apresentado por baixo da caixa de verificação. É um número aleatório criado para a sua cópia do JeffOffice, igual em todas as aplicações JeffOffice. Não revela nada sobre si. Clique em **Copiar** para o copiar. Se as estatísticas estiverem desativadas ou se ainda não tiver sido criado nenhum número, é apresentado **nenhum**.
- Para que as estatísticas que já temos sejam eliminadas, envie o seu número de instalação para info@jeffoffice.com **antes** de desativar as estatísticas. Desativá-las apaga o número do computador e, sem ele, não conseguimos saber quais são os seus registos.
- Para desativar as estatísticas definitivamente sem abrir o JeffPub, defina a variável de ambiente `JP_NO_TELEMETRY` com qualquer valor antes de o iniciar. Assim, nada é contabilizado nem enviado, e o JeffPub não faz a pergunta inicial.
- **O seu nome** é o mesmo nome que **Nome de utilizador**, acima.

## Relacionados

- [Recuperar trabalho não guardado](recover)
- [Verificar a ortografia e a hifenização](proofing)
- [Obter ajuda e comunicar problemas](support)
